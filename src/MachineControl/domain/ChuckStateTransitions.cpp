struct Loaded 
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to Unloaded state");
        return {};
    }

    GSL::state_transition_to<ReadyForSwap> on_state_transition(const transition_to_ReadyForSwap&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to ReadyForSwap state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
        return {};
    }
};

struct ReadyForSwap
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForSwap state with transition to Loaded state");
        return {};
    }

    GSL::state_transition_to<ReadyForUnloading> on_state_transition(const transition_to_ReadyForUnloading&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForSwap state with transition to ReadyForUnloading state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ReadyForSwap state!");
        return {};
    }
};

struct ReadyForUnloading
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ReadyForUnloading state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ReadyForUnloading state!");
        return {};
    }
};

struct Unloaded
{
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Loaded> on_state_transition(const transition_to_Loaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Unloaded state with transition to Loaded state");
        return {};
    }

    GSL::state_transition_to<ReadyForSwap> on_state_transition(const transition_to_ReadyForSwap&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Unloaded state with transition to ReadyForSwap state"); // Only for empty Expose station
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Unloaded state!");
        return {};
    }
};