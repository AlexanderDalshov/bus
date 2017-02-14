#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>

namespace detail {
static size_t nextTypeId = 0;
}

template <typename T>
size_t GetTypeId() {
  static const size_t id = detail::nextTypeId++;
  return id;
}

struct EventA {
  int x;
  explicit EventA(int x) : x(x) {}
};

struct EventB {
  std::string s;
  explicit EventB(std::string s) : s(s) {}
};


class EventBus;

class BusListener {
 public:

  virtual ~BusListener() {
    // remove all connections 
    /*
    for (auto connection: connections_) {
      auto bus = connection.lock();
      if (bus)
        bus->unsubscribe(this);
    }
    */
  }
  
 private:
  friend class EventBus;
  void connect(std::weak_ptr<EventBus> bus);
  void disconnect(std::weak_ptr<EventBus> bus);
  
 private:
  std::vector<std::weak_ptr<EventBus>> connections_; 
};

//
class EventBus {
 private:
  struct BaseListener {
    virtual ~BaseListener() {}
    virtual void operator()(const void*) = 0;
  };

 public:
  template <typename Event, class Listener>
  void subscribe(Listener* listener) {
    struct ListenerImpl : BaseListener {
      Listener* listener_;

      ListenerImpl(Listener* listener) : listener_(listener) {}

      void operator()(const void* event) override {
        const auto e = static_cast<const Event*>(event);
        (*listener_)(*e);
      }
    };

    auto& listeners = listeners_map_[GetTypeId<Event>()];
    listeners.push_back(std::make_shared<ListenerImpl>(listener));
  }

  template <typename Event, class Listener>
  void subscribe(Listener* listener, void (Listener::*method)(const Event&)) {
    struct ListenerImpl : BaseListener {
      typedef void (Listener::*Method)(const Event&);
      Listener* listener_;
      Method method_;

      ListenerImpl(Listener* listener, Method method) : listener_(listener), method_(method) {}

      void operator()(const void* event) override {
        const auto e = static_cast<const Event*>(event);
        (listener_->*method_)(*e);
      }
    };

    auto& listeners = listeners_map_[GetTypeId<Event>()];
    listeners.push_back(std::make_shared<ListenerImpl>(listener, method));
  }

  template <typename Event>
  void send(const Event& event) {
    if (is_sending_) {
      assert(0 && "TODO");
    } else {
      is_sending_ = true;
      auto id = GetTypeId<Event>();
      auto it = listeners_map_.find(id);
      if (it == listeners_map_.end()) {
        is_sending_ = false;
        return;
      }
      
      for (auto listener : it->second) {
        (*listener)(&event);
      }
      is_sending_ = false;

      // complete unsibscribe
    }
  }

  template <class Listener>
  void unsibscribe(Listener* listener) {
    if (is_sending_) {
      //
      assert(0 && "TODO");
    }
  }

 private:
  bool is_sending_ = false;
  std::unordered_map<size_t, std::vector<std::shared_ptr<BaseListener>>> listeners_map_;
};

//
struct Foo {
  void operator()(const EventA& a) { std::cout << "Foo::onA() " << a.x << std::endl; }

  void operator()(const EventB& b) { std::cout << "Foo::onB()" << b.s << std::endl; }
};

struct FooB {
  void operator()(const EventB& b) { std::cout << "Foo2::onB()" << b.s << std::endl; }
};

struct FooM {
  /*
  FooM(EvenBus* bus) : bus_(bus) {
    bus.subscribe<EventB>(this, &FooM::methodB);
  }

  ~FooM() {
    std::cout<<"FooM::~FooM()"<<std::endl;
    bus.unsibscribe(this);
  }
  */
  
  void methodB(const EventB& b) {
    std::cout << "FooM::methodB()" << b.s << std::endl;
    m = b.s;
    // bus.send(EventA(1)); // recursion - tobe implemented
  }
  
  EventBus* bus_;

  std::string m;
};

struct FooMFactory {

  std::shared_ptr<FooM> case1(EventBus* bus, bool hasA) {
    std::shared_ptr<FooM> fooM = std::make_shared<FooM>();
    // bus->subscribe<EventB>(fooM.get());
    if (hasA) {
      //...
    }
    return fooM;
  }
};
