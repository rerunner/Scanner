#pragma once

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
#include <nlohmann/json.hpp>

#include "GenLogger.hpp"
#include "Expose/application/Expose.hpp"

using json = nlohmann::json;

namespace Expose { namespace Application { namespace ExposeCommands {
  struct DummyMethod 
  {
      Verdi::Uuid waferId;
  };

  struct ExposeWafer 
  {
      Verdi::Uuid waferId;
  };

  using Command = std::variant<DummyMethod, ExposeWafer>;

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
    std::string message;

    // Kafka part
    std::unique_ptr<cppkafka::Configuration> kafkaConfig;
    std::unique_ptr<cppkafka::Producer> kafkaProducer;

    public:
    CommandExecutor(Application::Expose& expose) : expose_(expose),
                                                   name_("ExposeCommandProcessorQueue"), 
                                                   thread_cnt(4), // Depth
                                                   threads_(thread_cnt)
    {
      GSL::Dprintf(GSL::DEBUG, "Creating dispatch queue: ", name_.c_str());
      GSL::Dprintf(GSL::DEBUG, "Dispatch threads: ", thread_cnt);

      //! Create the Kafka config
      std::vector<cppkafka::ConfigurationOption> kafkaConfigOptions;
      cppkafka::ConfigurationOption exposeConfigOption{"metadata.broker.list", "localhost:9092"};
      kafkaConfigOptions.push_back(exposeConfigOption);
      kafkaConfig = std::make_unique<cppkafka::Configuration>(cppkafka::Configuration{kafkaConfigOptions});
      //! Create the Kafka producer
      kafkaProducer = std::make_unique<cppkafka::Producer>(*kafkaConfig);

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

    void operator()(const ExposeWafer& cmd)
    {
       dispatch([&] {
        GSL::Dprintf(GSL::INFO, "Expose Expose command execution start for waferId = ", cmd.waferId.Get());

        expose_.exposeWafer(cmd.waferId);

        GSL::Dprintf(GSL::DEBUG, "Expose Expose command execution completed, sending Kafka message");
        json jMessage;
        jMessage.emplace("Message", "CommandCompleted");
        jMessage.emplace("Command", "ExposeWafer");
        jMessage.emplace("Id", cmd.waferId.Get());
        // serialize to CBOR
        std::vector<std::uint8_t> message = json::to_cbor(jMessage);
        cppkafka::Buffer bmess(message); // Make sure the kafka message is using the cbor binary format and not a string
        kafkaProducer->produce(cppkafka::MessageBuilder("exposeTopic").partition(0).payload(bmess));

        //kafkaProducer->flush(std::chrono::milliseconds(30000)); // 30s timeout
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
      expose_.dummyMethod(cmd.waferId);
    }
    // Deleted operations for Command queue processor part
    CommandExecutor(const CommandExecutor& rhs) = delete;
    CommandExecutor& operator=(const CommandExecutor& rhs) = delete;
    CommandExecutor(CommandExecutor&& rhs) = delete;
    CommandExecutor& operator=(CommandExecutor&& rhs) = delete;

  private:
    Application::Expose& expose_;
  };
}}} // namespaces
