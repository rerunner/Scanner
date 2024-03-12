
#include "MachineControl.hpp"
#include "Measure/Leveling/application/LevelingCommand.hpp"


namespace MachineControl
{
    void MachineControl::Initialize()
    {
        machineControlStateMachine.on_state_transition(transition_to_Idle{});
    }

    void MachineControl::Execute()
    {
        machineControlStateMachine.on_state_transition(transition_to_Executing{});

        Leveling::Leveling leveling; // Create leveling object
        LevelingCommands::CommandExecutor executor(leveling); // Give commands to leveling object

        //actual execution, can be moved to queue command processor
        LevelingCommands::Command command; 
        command = LevelingCommands::MeasureWafer{"1"}; // Command: Measure Wafer number 1
        std::visit(executor, command); // execute the command
    }
}
