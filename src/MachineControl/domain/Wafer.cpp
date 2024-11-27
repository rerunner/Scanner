#include <nlohmann/json.hpp>
#include "Wafer.hpp"

namespace MachineControlContext {

std::shared_ptr<Uuid> Wafer::GetLotId() 
{
    return parentLot_;
}

} // namespace MachineControlContext

// Boilerplate
HIBERLITE_EXPORT_CLASS(MachineControlContext::Wafer)
