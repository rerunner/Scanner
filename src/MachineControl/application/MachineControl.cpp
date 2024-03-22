
#include "MachineControl.hpp"
#include "Measure/Leveling/application/LevelingCommand.hpp"
#include "Expose/Expose/application/Expose.hpp"
#include "domain/Wafer.hpp"


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

        Leveling::Leveling leveling; // Create leveling object
        LevelingCommands::CommandExecutor executor(leveling); // Give commands to leveling object

        //actual execution, can be moved to queue command processor
        LevelingCommands::Command command; 
        command = LevelingCommands::MeasureWafer{newWafer.GetId()}; // Command: Measure Wafer with uuid
        std::visit(executor, command); // execute the command
    }
}
