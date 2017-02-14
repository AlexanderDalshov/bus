#include <iostream>

#include "core.hpp"
// #include "event_manager.hpp"

namespace gps {
class GpsInterface : public SystemInterface<GpsInterface> {
 public:
  virtual void sendPos(int x) = 0;
};

struct GPSPositionEvent {
  int x;
  explicit GPSPositionEvent(int x) : x(x) {}
};

}  // gps

namespace gps {
namespace impl {

class GpsSystemImpl : public GpsInterface {
 public:
  void sendPos(int x) override {
    std::cout<<"GPSImpl::send gps pos "<<x<<std::endl;
    core()->eventManager().send(GPSPositionEvent(x));
  }
};

class GpsSystemImpl2 : public GpsInterface {
 public:
  void sendPos(int) override {}
};
}
}  // gps impl

class RouteInterface : public SystemInterface<RouteInterface> {
 public:
  virtual void foo() = 0;
};

class RouteSystemImpl : public RouteInterface {
 public:
  void foo() override { std::cout << "RouteImpl::foo()" << std::endl; }

 protected:
  bool configure(Core* core) override {
    auto& em = core->eventManager();
    std::cout<<"RouteImpl::subscribe on gps events"<<std::endl;
    em.subscribe<gps::GPSPositionEvent>(this, &RouteSystemImpl::onGPSPosition);
    return true;
  }

  void onGPSPosition(const gps::GPSPositionEvent& gps_position) {
    std::cout<<"Route::onGPSPosition "<< gps_position.x << std::endl;
  }
};

int main(int, char* []) {
  Core core;
  // register systems
  {
    core.addSystem(std::make_shared<gps::impl::GpsSystemImpl>());

    // don't allow to register same interface twice
    auto success = core.addSystem(std::make_shared<gps::impl::GpsSystemImpl2>());
    assert(success == false);

    core.addSystem(std::make_shared<RouteSystemImpl>());
    //
    core.configure();
  }

  //
  auto route = core.getSystem<RouteInterface>();
  assert(route);
  route->foo();

  // process gps 
  {
    auto gps = core.getSystem<gps::GpsInterface>();
    assert(gps);
    gps->sendPos(5);
    gps->sendPos(15);
    gps->sendPos(42);
  }
  
  std::cout << "done" << std::endl;

  return 0;
}

//
int main2(int argc, char* argv[]) {
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
  //
  {
    FooM fm;
    bus.subscribe<EventB>(&fm, &FooM::methodB);
    // CRASH: we didn't unsubscribe
    //    bus.unsubscibe(&fm);
  }
  // bus.subscribe<EventA>(&fb); // not possible to compile - unsupported type but this listener

  bus.send(EventB("Hello"));
  bus.send(EventB("World! World World World"));
  return 0;
}
