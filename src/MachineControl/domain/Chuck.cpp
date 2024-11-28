#include "Chuck.hpp"

namespace MachineControlContext {

std::shared_ptr<Uuid> Chuck::GetWaferId() 
{   return holdingWaferId_;
}

void Chuck::LoadWafer(Uuid wId)
{   holdingWaferId_ = std::make_unique<Uuid>(wId);
    chuckStateMachine.on_state_transition(chuckState::transition_to_Loaded{});
    state = "Loaded";
}

void Chuck::UnloadWafer()
{   holdingWaferId_ = nullptr;
    chuckStateMachine.on_state_transition(chuckState::transition_to_Unloaded{});
    state = "Unloaded";
}

void Chuck::SetReadyForSwap()
{   chuckStateMachine.on_state_transition(chuckState::transition_to_ReadyForSwap{});
    state = "ReadyForSwap";
}

StationEnumType Chuck::GetStation(void)
{   return atStation_;
}

void Chuck::SetStation(StationEnumType newStation)
{   atStation_ = newStation;
}

void Chuck::SwapStation()
{   if (state == "ReadyForSwap")
    {   if (atStation_ == StationEnumType::MeasureStation)
        {   atStation_ = StationEnumType::ExposeStation;
            chuckStateMachine.on_state_transition(chuckState::transition_to_Loaded{});
            state = "Loaded";
        }
        else
        {   atStation_ = StationEnumType::MeasureStation;
            chuckStateMachine.on_state_transition(chuckState::transition_to_ReadyForUnloading{});
            state = "ReadyForUnloading"; // NOK for empty chuck
        }
    }
    else
    {   GSL::Dprintf(GSL::FATAL, "Attempt to swap chuck while chuck not ready for swap");
    }
}

} // namespace MachineControlContext 

// Boilerplate
HIBERLITE_EXPORT_CLASS(MachineControlContext::Chuck)
