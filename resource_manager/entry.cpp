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

behavior testRegisteringSystem( caf::event_based_actor* self, actor registering_service )
{
  std::this_thread::sleep_for( std::chrono::seconds(10) );
  self->send(registering_service, lfge::core::find_service_v, "test");
  return {
    [=]( std::string id, caf::actor_addr addr )
    {
      actor act = actor_cast<actor>(addr);
      std::stringstream ss;
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::time_t now_c = std::chrono::system_clock::to_time_t(now);
      std::tm now_tm = *std::localtime(&now_c);
      char buff[70];
      strftime(buff, sizeof buff, "%A %c", &now_tm);
      ss << "Hi at " << buff;

      self->send( act, lfge::core::string_data_atom_v, ss.str() );
    },
    after( std::chrono::seconds( 5 ) ) >> [=]()
    {
      self->send(registering_service, lfge::core::find_service_v, "test");
    }
  };
}

void caf_main(actor_system& system, const caf::actor_system_config& cfg) {

  auto port = get_or(system.config(), "port", uint16_t{4555});
  auto res = system.middleman().publish_local_groups(port);
  if (!res) {
    std::cerr << "*** publishing local groups failed: "
              << to_string(res.error()) << std::endl;
    return;
  }
  std::cout << "*** listening at port " << *res << std::endl;
  auto regservice = system.spawn<lfge::resource_manager::RegisteringService>(  );
  system.spawn(testRegisteringSystem, regservice);
}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, io::middleman)