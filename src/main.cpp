#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

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
  explicit EventA(int x) :x(x) {}
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
      Listener * listener_;

      ListenerImpl(Listener* listener) : listener_(listener) {}
      
      void operator()(const void* event) override {
        const auto e = reinterpret_cast<const Event*>(event);
        (*listener_)(*e);
      }
    };

    auto& listeners = listeners_map_[GetTypeId<Event>()];
    listeners.push_back(std::make_shared<ListenerImpl>(listener));
  }

  template<typename Event>
  void send(const Event& event) {
    auto id = GetTypeId<Event>();
    auto it = listeners_map_.find(id);
    if (it == listeners_map_.end())
      return;
    for (auto listener : it->second) {
      (*listener)(&event);
    }
  }

 private:
  std::unordered_map<size_t, std::vector<std::shared_ptr<BaseListener>>> listeners_map_;
};

//
struct Foo {

  void operator()(const EventA& a) {
    std::cout<<"Foo::onA() " << a.x <<std::endl;
  }
  
  void operator()(const EventB& b) {
    std::cout<<"Foo::onB()" << b.s <<std::endl;
  }
};

struct FooB{
  void operator()(const EventB& b) {
    std::cout<<"Foo2::onB()" << b.s <<std::endl;
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
  
  bus.subscribe<EventB>(&f);
  bus.subscribe<EventB>(&fb);
  // bus.subscribe<EventA>(&fb); // not possible to compile - unsupported type but this listener

  bus.send(EventB("Hello"));
  bus.send(EventB("World!"));
  return 0;
}
