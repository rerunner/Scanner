#include <nlohmann/json.hpp>
#include "FiniteStateMachine.hpp"
#include "GenLogger.hpp"

#include "Lot.hpp"

namespace MachineControlContext {

void Lot::AddWafer(Uuid wId)
{
    waferIds_.push_back(wId);
}

void Lot::RemoveWafer(Uuid wId)
{
    waferIds_.remove(wId);
}

} // namespace MachineControlContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(MachineControlContext::Lot)
