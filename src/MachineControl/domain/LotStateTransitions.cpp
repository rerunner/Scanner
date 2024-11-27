
struct Loaded 
{
  void on_update() const {
  GSL::Dprintf(GSL::DEBUG, "we are running!");
  }

  GSL::state_transition_to<Started> on_state_transition(const transition_to_Started&) const {
      GSL::Dprintf(GSL::DEBUG, "Leaving Loaded state with transition to Started state");
      return {};
  }

  template<typename Transition>
  GSL::invalid_state_transition on_state_transition(const Transition&) const {
      GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Loaded state!");
      return {};
  }
};
struct Started
{
  void on_update() const {
  GSL::Dprintf(GSL::DEBUG, "we are running!");
  }

  GSL::state_transition_to<Finished> on_state_transition(const transition_to_Finished&) const {
      GSL::Dprintf(GSL::DEBUG, "Leaving Prealigned state with transition to Finished state");
      return {};
  }

  template<typename Transition>
  GSL::invalid_state_transition on_state_transition(const Transition&) const {
      GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Started state!");
      return {};
  }
};
struct Finished
{
  void on_update() const {
  GSL::Dprintf(GSL::DEBUG, "we are running!");
  }

  GSL::state_transition_to<Unloaded> on_state_transition(const transition_to_Unloaded&) const {
      GSL::Dprintf(GSL::DEBUG, "Leaving Measured state with transition to Unloaded state");
      return {};
  }

  template<typename Transition>
  GSL::invalid_state_transition on_state_transition(const Transition&) const {
      GSL::Dprintf(GSL::ERROR, "State transition: ", typeid(Transition).name(), " is not supported in Finished state!");
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
