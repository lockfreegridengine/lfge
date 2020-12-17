#include <string>
#include <iostream>
#include <string>
#include <iostream>

#include "caf/all.hpp"
#include <chrono>
#include<algorithm>
#include <thread>
#include "core/Heartbeat.hpp"
#include "core/DataContainer.hpp"
#include <unordered_map>
#include <random>
#include <set>
#include <vector>


using std::endl;
using std::string;

using namespace caf;
using namespace lfge::core;

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
  auto id =  self->spawn<lfge::core::HeartbeatReciever>("toto", 
    []( lfge::core::HeartbeatReciever&, const std::size_t & index ) { std::cout << "HB " << index << "\n"; std::this_thread::sleep_for( std::chrono::seconds(20) ); }, 
    10000,
    [](lfge::core::HeartbeatReciever&) {  std::cout << "HB RECIEVER TIMEOUT" << std::endl;  }
    );

  return [=]( lfge::core::heartbeat_atom, const std::size_t& messageIndex )
  {
    //self->request(id, lfge::core::heartbeat_atom_v, messageIndex);
  };
}

std::default_random_engine generator[10];

std::string randomString( const std::size_t &index, const std::size_t& size )
{
  std::uniform_int_distribution<char> distribution(0,25);
  std::string str;
  str.resize(size);
  std::generate(str.begin(), str.end(), [&distribution, &generator, &index](){ return 'A' + distribution(generator[index]); });
  return str;
}

behavior containerAdder(event_based_actor* self, caf::actor container, std::size_t index)
{
  self->send( container, read_atom_v, 0 );
  std::uniform_int_distribution<int> distribution(0,1000);
  int current = distribution(generator[index]);
  self->send(container, write_atom_v, current, randomString(index,10));
  self->send(container, read_atom_v, current);
  return [self, container, index, &generator]( const int& i, const std::string& s )
  {
    //std::cout << "Key " << i << " has value " << s << std::endl;
    std::uniform_int_distribution<int> distribution(0,1000);
    int current = distribution(generator[index]);

    self->send(container, write_atom_v, current, randomString(index,10));
    self->send(container, read_atom_v, current);
  };

}

behavior keysChecker( blocking_actor* self, caf::actor container )
{
  std::default_random_engine gen;
  std::uniform_int_distribution<int> distribution(0,1000);

  while(1)
  {
    int current = distribution(gen);
    self->send( container, contains_atom_v, current );
    bool contains = false;
    self->receive( [&contains]( bool c ){ contains = c; } );
    if( contains )
    {
      //std::cout << "Container contains key : " << current << std::endl;
      self->send(container, read_atom_v, current);
      self->receive( []( const int& i, const std::string& v ){
        //std::cout << "Key " << i << " has value " << v << std::endl;
      } );
    }
    else
    {
      //std::cout << "Container does not contain key "<< current << std::endl; 
    }
    
  }

}

void caf_main(actor_system& system) {
  // create a new actor that calls 'mirror()'
  auto mirror_actor = system.spawn(mirror);
  // create another actor that calls 'hello_world(mirror_actor)';
  system.spawn(hello_world, mirror_actor);
  // system will wait until both actors are destroyed before leaving main
  //system.spawn(counter);

  auto hb_replier = system.spawn<lfge::core::HeartbeatReciever>("toto", 
    []( lfge::core::HeartbeatReciever&, const std::size_t & index ) { std::cout << "HB " << index << "\n"; std::this_thread::sleep_for( std::chrono::seconds(5) ); }, 
    10000,
    [](lfge::core::HeartbeatReciever&) {  std::cout << "HB RECIEVER TIMEOUT" << std::endl;  }
    );
  system.spawn<lfge::core::HeartbeatSender>( hb_replier, 5000, 1000, 3, "MyActor");


  auto container = system.spawn< lfge::core::DataContainerMapActor<int, std::string> >();
  for( int i = 0; i< 10; ++i )
    system.spawn(containerAdder, container, i);
  system.spawn(keysChecker, container);

  auto sA = system.spawn< lfge::core::DataContainerActor<int, std::set<int>> >();
  auto va = system.spawn< lfge::core::DataContainerActor<int, std::vector<int>> >();
}

// creates a main function for us that calls our caf_main
CAF_MAIN(caf::id_block::lfge_core)