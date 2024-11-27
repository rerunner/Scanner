#include "Station.hpp"

namespace MachineControlContext {

void Station::DoCommand()
{   if (commandCompleted) 
    {   commandCompleted = false;
    } else 
    {   GSL::Dprintf(GSL::FATAL, "Previous Command not yet completed!");
    }
}

void Station::CommandHasCompleted()
{   commandCompleted = true;
} // To Be Removed

bool Station::GetCommandCompletedState()
{   return commandCompleted;
}

} // namespace MachineControlContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(MachineControlContext::Station)
