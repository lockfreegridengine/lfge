#include <string>
#include <iostream>
#include <string>
#include <iostream>

#include "caf/all.hpp"
#include <chrono>
#include<algorithm>
#include <thread>
#include "Heartbeat.hpp"

using std::endl;
using std::string;

using namespace caf;

behavior mirror(event_based_actor* self) {
  // return the (initial) actor behavior
  return {
    // a handler for messages containing a single string
    // that replies with a string
    [=](const string& what ) -> string {
      // prints "Hello World!" via aout (thread-safe cout wrapper)
      //aout(self) << what << endl;
      // reply "!dlroW olleH"
      return string(what.rbegin(), what.rend()) ;
    }
  };
}
static std::atomic<uint64_t> count = 0;
behavior hello_world(event_based_actor* self, const actor& buddy) {
    self->send(buddy, "Hello");
  // send "Hello World!" to our buddy ...
  return [&](const string& what) -> string {
      //aout(self) << what << endl;
      ++count;
      return what;
  };
}

behavior counter(event_based_actor* self)
{
    while(1)
    {
        count = 0;
        std::this_thread::sleep_for( std::chrono::seconds(1) );
        aout(self) << count << "\n";
    }
}

behavior heartbeatReplier(event_based_actor* self)
{
  return [=]( lfge::core::heartbeat_atom, const std::size_t& messageIndex )
  {
    std::this_thread::sleep_for( std::chrono::milliseconds(3500) );
    return make_result( lfge::core::heartbeat_atom_v, messageIndex );
  };
}

void caf_main(actor_system& system) {
  // create a new actor that calls 'mirror()'
  auto mirror_actor = system.spawn(mirror);
  // create another actor that calls 'hello_world(mirror_actor)';
  system.spawn(hello_world, mirror_actor);
  // system will wait until both actors are destroyed before leaving main
  system.spawn(counter);

  auto hb_replier = system.spawn(heartbeatReplier);

  system.spawn<lfge::core::HeartbeatSender>( hb_replier, 10000, 5000, 3, "MyActor", 
  [&]( lfge::core::HeartbeatSender& )
  {
    std::cout << "N Timeouts" << std::endl;
  },
  [&]( const lfge::core::HeartbeatSender& )
  {
    std::cout << "On HB" << std::endl;
  },
  [&]( const lfge::core::HeartbeatSender& )
  {
    std::cout << "Timeout" << std::endl;
  },
  [&]( const lfge::core::HeartbeatSender& )
  {
    std::cout << "Started" << std::endl;
  } );
}

// creates a main function for us that calls our caf_main
CAF_MAIN(caf::id_block::lfge_core)