#pragma once

#include "event_manager.hpp"

class Core;

class SystemBase {
  friend class Core;
 protected:
  virtual bool configure(Core* core) { return true; }
  Core* core() const { return core_;}

 private:
  Core* core_;
};

template <class T>
class SystemInterface : public SystemBase {
 public:
  using Interface_t = T;
};

class Core {
 public:
  template <class T>
  bool addSystem(std::shared_ptr<T> system) {
    auto base_system = std::static_pointer_cast<SystemBase>(system);
    auto interface_id = GetTypeId<typename T::Interface_t>();

    bool result;
    std::tie(std::ignore, result) = systems_.insert(std::make_pair(interface_id, base_system));
    return result;
  }

  template <class T>
  std::shared_ptr<T> getSystem() {
    auto interface_id = GetTypeId<typename T::Interface_t>();
    auto it = systems_.find(interface_id);
    std::shared_ptr<T> result;
    if (it != systems_.end())
      result = std::static_pointer_cast<T>(it->second);
    return result;
  }

  void configure() {
    for (auto rec : systems_) {
      auto system = rec.second;
      system->core_ = this;
      system->configure(this);
    }
  }

  void shutdown();  // TODO: ?

  EventBus& eventManager() { return event_manager_; }

 private:
  EventBus event_manager_;
  std::unordered_map<size_t, std::shared_ptr<SystemBase>> systems_;
};
