struct Loaded 
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<PreAligned> on_state_transition(const transition_to_PreAligned&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to PreAligned state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
        return {};
    }
};
struct PreAligned
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Measured> on_state_transition(const transition_to_Measured&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving PreAligned state with transition to Measured state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in PreAligned state!");
        return {};
    }
};
struct Measured
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<ApprovedForExpose> on_state_transition(const transition_to_ApprovedForExpose&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Measured state with transition to ApprovedForExpose state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Measured state!");
        return {};
    }
};
struct ApprovedForExpose
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Exposed> on_state_transition(const transition_to_Exposed&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving ApprovedForExpose state with transition to Exposed state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in ApprovedForExpose state!");
        return {};
    }
};
struct Exposed
{
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Exposed state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Exposed state!");
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

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Unloaded state!");
        return {};
    }
};
struct Rejected
{ // Dummy for now
    void on_update() const {
        GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Rejected state with transition to Unloaded state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Rejected state!");
        return {};
    }
};