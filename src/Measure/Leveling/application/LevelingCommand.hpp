#ifndef LEVELINGCOMMAND_H
#define LEVELINGCOMMAND_H

#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <variant>        // std::variant
#include <sstream>        // std::stringstream
#include <functional>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>

#include <cppkafka/cppkafka.h>

#include "Uuid.hpp"
#include "GenLogger.hpp"
#include "Leveling.hpp"
#include "domain/WaferHeightMap.hpp"

namespace LevelingCommands
{
  struct DummyMethod 
  {
    Uuid waferId;
  };

  struct MeasureWafer 
  {
    Uuid waferId;
  };

  using Command = std::variant<DummyMethod, MeasureWafer>;

  class CommandExecutor
  {
    private:
    // Command queue processor part
    typedef std::function<void(void)> fp_t;
    size_t thread_cnt;
    std::string name_;
    std::mutex lock_;
    std::vector<std::thread> threads_;
    std::queue<fp_t> q_;
    std::condition_variable cv_;
    bool quit_ = false;
    
    // Kafka part
    std::unique_ptr<cppkafka::Configuration> kafkaConfig;
    std::unique_ptr<cppkafka::Producer> kafkaProducer;

    public:
    CommandExecutor(Leveling::Application::Leveling& leveling) : leveling_(leveling), 
                                                                 name_("LevelingCommandProcessorQueue"), 
                                                                 thread_cnt(4), // Depth
                                                                 threads_(thread_cnt)
    {
      GSL::Dprintf(GSL::DEBUG, "Creating dispatch queue: ", name_.c_str());
      GSL::Dprintf(GSL::DEBUG, "Dispatch threads: ", thread_cnt);

      for(size_t i = 0; i < threads_.size(); i++)
      {
        threads_[i] = std::thread(&CommandExecutor::dispatch_thread_handler, this);
        threads_[i].detach();
      }
    }
    /// @brief 
    ~CommandExecutor()
    {
      GSL::Dprintf(GSL::DEBUG, "CommandExecutor destructor");
      GSL::Dprintf(GSL::DEBUG, "Destroying dispatch threads...");

      // Signal to dispatch threads that it's time to wrap up
      std::unique_lock<std::mutex> lock(lock_);
      quit_ = true;
      cv_.notify_all();
      lock.unlock();

      // Wait for threads to finish before we exit
      for(size_t i = 0; i < threads_.size(); i++)
      {
        if(threads_[i].joinable())
        {
          GSL::Dprintf(GSL::DEBUG, "Destructor: Joining thread", i, "until completion");
          threads_[i].join();
        }
      }
    }

    void dispatch(const fp_t& op)
    {
      std::unique_lock<std::mutex> lock(lock_);
      q_.push(op);
      cv_.notify_one();
    }

    void dispatch(fp_t&& op)
    {
      std::unique_lock<std::mutex> lock(lock_);
      q_.push(std::move(op));
      cv_.notify_one();
    }

    void operator()(const MeasureWafer& cmd)
    {
      dispatch([&] {
        GSL::Dprintf(GSL::DEBUG, "Leveling command execution start for waferId = ", cmd.waferId.Get());
      
        //! Create the Kafka config
        std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
        cppkafka::ConfigurationOption levelingConfigOption{"metadata.broker.list", "localhost:9092"};
        kafkaConfigOptions.push_back(levelingConfigOption);
        kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
        //! Create the Kafka producer
        kafkaProducer = std::make_unique<cppkafka::Producer>(*kafkaConfig);

		    leveling_.measureWafer(cmd.waferId);
      
        GSL::Dprintf(GSL::DEBUG, "Leveling command executed in async mode, sending Kafka message to indicate completion");
        //! Produce a Kafka event message for command completion
        std::stringstream smessage;
        smessage << "MeasureWaferCompleted:" << cmd.waferId.Get();
        std::string message = smessage.str();
        kafkaProducer->produce(cppkafka::MessageBuilder("levelingTopic").partition(0).payload(message));
        kafkaProducer->flush();
	    });
    }

    void dispatch_thread_handler(void)
    {
      std::unique_lock<std::mutex> lock(lock_);

      do
      {
        // Wait until we have data or a quit signal
        cv_.wait(lock, [this] {return (q_.size() || quit_);});

        // after wait, we own the lock
        if(!quit_ && q_.size())
        {
          auto op = std::move(q_.front());
          q_.pop();
          // unlock now that we're done messing with the queue
          lock.unlock();

          op(); // Actual execution of operation

          lock.lock();
        }
      } while(!quit_);
    }

    void operator()(const DummyMethod& cmd)
    {
      leveling_.dummyMethod(cmd.waferId);
    }
    // Deleted operations for Command queue processor part
    CommandExecutor(const CommandExecutor& rhs) = delete;
    CommandExecutor& operator=(const CommandExecutor& rhs) = delete;
    CommandExecutor(CommandExecutor&& rhs) = delete;
    CommandExecutor& operator=(CommandExecutor&& rhs) = delete;

  private:
    Leveling::Application::Leveling& leveling_;
  };
}

#endif