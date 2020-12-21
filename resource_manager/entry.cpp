#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include <chrono>
#include<algorithm>
#include "core/DataContainer.hpp"
#include "RegisteringService.hpp"
#include <thread>
#include <sstream>

using namespace caf;
using namespace lfge::resource_manager;

using typed_tester = typed_actor<
          result<void>( lfge::core::start_atom )//,
          //result<void>( caf::error )
        >;

typed_tester::behavior_type testRegisteringSystem(typed_tester::pointer self, typed_registration_actor_hdl registering_service )
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
      self->request( registering_service, std::chrono::seconds(100), lfge::core::find_service_v, "test" ).then(
        [&]( std::string name, caf::actor_addr addr )
        {
          actor act = actor_cast<actor>(addr);
          std::stringstream ss;
          std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
          std::time_t now_c = std::chrono::system_clock::to_time_t(now);
          std::tm now_tm = *std::localtime(&now_c);
          char buff[70];
          strftime(buff, sizeof buff, "%A %c", &now_tm);
          ss << "Hi at " << buff;

          self->anon_send( act, lfge::core::string_data_atom_v, ss.str() );
        },
        [&](caf::error e)
        {
          aout(self) << " got error " << to_string(e) << std::endl;
        }
      );
    },
    // [=]( caf::error e)
    // {
    //   aout(self) << "Got error " << to_string(e) << std::endl;
    // },
    after( std::chrono::seconds( 5 ) ) >> [=]()
    {
      self->send(self, lfge::core::start_atom_v);
    }
  };
}



//Blocking test
/*
void testRegisteringSystem( blocking_actor *self, typed_registration_actor_hdl registering_service )
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

void caf_main(actor_system& system, const caf::actor_system_config& cfg) {

  auto port = get_or(system.config(), "port", uint16_t{4555});
  auto res = system.middleman().publish_local_groups(port);
  if (!res) {
    std::cerr << "*** publishing local groups failed: "
              << to_string(res.error()) << std::endl;
    return;
  }
  std::cout << "*** listening at port " << *res << std::endl;
  auto regservice = system.spawn<RegisteringService>(  );
  system.spawn(testRegisteringSystem, regservice);
}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, io::middleman)