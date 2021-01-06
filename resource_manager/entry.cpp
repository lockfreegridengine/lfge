#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include <chrono>
#include<algorithm>
#include "resource_manager/registering_service.hpp"
#include <thread>
#include <sstream>

#include "core/logger.hpp"
#include "resource_manager/treatment.hpp"

using namespace caf;
using namespace lfge::resource_manager;
using namespace lfge::core;

using typed_tester = typed_actor<
          result<void>( lfge::core::start_atom )//,
          //result<void>( caf::error )
        >;

typed_tester::behavior_type testDistribSystem(typed_tester::pointer self, simple_int_replyer replier )
{
  std::this_thread::sleep_for( std::chrono::seconds(10) );
  //self->send(registering_service, lfge::core::find_service_v, "test");
  //self->set_default_handler( caf::print_and_drop );
   self->attach_functor([=](const error& reason) {
     aout(self) << " exited: " << to_string(reason) << std::endl;
   });
   

   self->send(self, lfge::core::start_atom_v);

  return {
    [=]( lfge::core::start_atom )
    {
      self->request( replier, caf::infinite, "test" ).then(
        [=](int reply)
        {
          //aout(self) << reply << std::endl;
        }

      );
    },
    // [=]( caf::error e)
    // {
    //   aout(self) << "Got error " << to_string(e) << std::endl;
    // },
    after( std::chrono::nanoseconds( 5 ) ) >> [=]()
    {
      self->send(self, lfge::core::start_atom_v);
    }
  };
}



//Blocking test
/*
void testRegisteringSystem( blocking_actor *self, typed_registration_actor registering_service )
{
  std::this_thread::sleep_for( std::chrono::seconds(10) );
  while(1)
  {
    actor_addr addr = nullptr;
    std::string name = "", which;
    bool error = false;
    ServiceName sname = "test";
    self->request( registering_service, std::chrono::seconds(100), lfge::core::find_service_v, sname ).receive(
      [&]( std::string n, caf::actor_addr a )
      {
        addr = a;
        name = std::move(n);
      },
      [&](caf::error e)
      {
        error = true;
        which = to_string(e);
      }
    );

    if(error)
    {
      aout(self) << " got error " << which << std::endl;
      continue;
    }

    actor act = actor_cast<actor>(addr);
    std::stringstream ss;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    char buff[70];
    strftime(buff, sizeof buff, "%A %c", &now_tm);
    ss << "Hi at " << buff;

    self->anon_send( act, lfge::core::string_data_atom_v, ss.str() );
    std::this_thread::sleep_for( std::chrono::seconds(5) );
  }
}*/

using typed_actor_t1 = caf::typed_actor< caf::result<void>(int) >;

using typed_actor_t2 = caf::typed_actor< caf::result<int>(std::string) >;

int i =0;

typed_actor_t2::behavior_type t2_impl ( typed_actor_t2::pointer p )
{
  return [=,&i](std::string s) -> caf::result<int>
  {
    aout(p) << "got " << s << std::endl;
    return ++i;
  };
}

typed_actor_t1::behavior_type t1_impl ( typed_actor_t1::pointer p )
{
  auto t2P = p->spawn(t2_impl);
  p->send( t2P, std::string("what") );
  return [=](int i)
  {
    aout(p) << "got " << i << std::endl;
    auto sender = p->current_sender();
    typed_actor_t2 act = caf::actor_cast<typed_actor_t2>(sender);
    p->send( act, std::string("what") );
  };
}

behavior logListener( caf::event_based_actor* self )
{
  auto grp = lfge::core::logger::getGroup();
  if(grp)
  {
    self->join(*grp);
  }
  return [=]( const loglevel& level, const std::string& str )
  {
    aout(self) << str << std::endl;
  };
}



void caf_main(actor_system& system, const caf::actor_system_config& cfg) {

  lfge::core::logger::init(system);
  //system.spawn(t1_impl);

  auto port = get_or(system.config(), "port", uint16_t{4555});
  auto res = system.middleman().publish_local_groups(port);
  if (!res) {
    std::cerr << "*** publishing local groups failed: "
              << to_string(res.error()) << std::endl;
    return;
  }
  std::cout << "*** listening at port " << *res << std::endl;
  auto regservice = system.spawn<registering_service>(  );
  //system.spawn(testRegisteringSystem, regservice);
  //system.spawn(logListener);

  auto staskdist = system.spawn( SimpleTaskDistributor, "test", regservice );
  system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, io::middleman)