struct Idle
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Processing> on_state_transition(const transition_to_Processing&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Idle state with transition to Processing state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Idle state!");
        return {};
    }
  };

  struct Processing
  {
    void on_update() const {
    GSL::Dprintf(GSL::DEBUG, "we are running!");
    }

    GSL::state_transition_to<Idle> on_state_transition(const transition_to_Idle&) const {
        GSL::Dprintf(GSL::DEBUG, "Leaving Processsing state with transition to Idle state");
        return {};
    }

    template<typename Transition>
    GSL::invalid_state_transition on_state_transition(const Transition&) const {
        GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Processing state!");
        return {};
    }
  };
  