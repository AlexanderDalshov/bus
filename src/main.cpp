#include <iostream>
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
  void methodB(const EventB& b) {
    std::cout << "FooM::methodB()" << b.s << std::endl;
    // bus.send(EventA(1)); // recursion - tobe implemented
  }
};

//
int main(int argc, char* argv[]) {
  std::cout << "EventA id:" << GetTypeId<EventA>() << std::endl;
  std::cout << "EventA id:" << GetTypeId<EventA>() << std::endl;
  std::cout << "EventA id:" << GetTypeId<EventA>() << std::endl;
  std::cout << "EventB id:" << GetTypeId<EventB>() << std::endl;
  std::cout << "EventA id:" << GetTypeId<EventA>() << std::endl;

  EventBus bus;
  Foo f;
  bus.subscribe<EventA>(&f);

  //
  bus.send(EventA(5));
  bus.send(EventA(42));
  bus.send(EventB("bad"));

  FooB fb;
  FooM fm;

  bus.subscribe<EventB>(&f);
  bus.subscribe<EventB>(&fb);
  bus.subscribe<EventB>(&fm, &FooM::methodB);
  // bus.subscribe<EventA>(&fb); // not possible to compile - unsupported type but this listener

  bus.send(EventB("Hello"));
  bus.send(EventB("World!"));
  return 0;
}
