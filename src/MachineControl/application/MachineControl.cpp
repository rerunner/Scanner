
#include "MachineControl.hpp"
//#include "Measure/Leveling/application/LevelingCommand.hpp"
//#include "Expose/Expose/application/Expose.hpp"
#include "domain/Wafer.hpp"

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

using namespace curlpp::options;

namespace MachineControl
{
    void MachineControl::Initialize()
    {
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }

    void MachineControl::Execute()
    {
        machineControlStateMachine.on_state_transition(transition_to_Executing{});

        Wafer newWafer; // load new wafer
        newWafer.PreAligned(); // Move wafer to prealigned state (will be event later)

        // Leveling part
        {
            std::cout << "starting leveling measure heightmap command with curl" << std::endl;
            curlpp::Cleanup myCleanup; // RAII cleanup
            curlpp::Easy levelingRequest;
            std::ostringstream urlCommand;
            urlCommand << "http://127.0.0.1:8003//measure/leveling/measure/" << newWafer.GetId();
            levelingRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
            levelingRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
            levelingRequest.perform();
            std::cout << std::endl << "MachineControl::Execute() -> finished leveling measure heightmap command." << std::endl;
            newWafer.Measured();
        }

        newWafer.ApprovedForExpose();

        // Expose part
        {
            std::cout << "starting expose command with curl" << std::endl;
            curlpp::Cleanup myCleanup; // RAII cleanup
            curlpp::Easy exposeRequest;
            std::ostringstream urlCommand;
            urlCommand << "http://127.0.0.1:8002/expose/expose/" << newWafer.GetId();
            exposeRequest.setOpt(curlpp::Options::Url(std::string(urlCommand.str())));
            exposeRequest.setOpt(curlpp::Options::CustomRequest("PUT"));
            exposeRequest.perform();
            std::cout << std::endl << "MachineControl::Execute() -> finished expose command." << std::endl;
            newWafer.Exposed();
        }

        newWafer.Unloaded();
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }
}
