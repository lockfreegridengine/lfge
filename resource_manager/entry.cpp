#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include <chrono>
#include<algorithm>
#include "resource_manager/registering_service.hpp"
#include <thread>
#include <sstream>
#include <numeric>

#include "core/logger.hpp"
#include "resource_manager/treatment.hpp"

using namespace caf;
using namespace lfge::resource_manager;
using namespace lfge::core;
using namespace lfge::resource_manager::task_distribution;

using typed_tester = typed_actor<
          result<void>( lfge::core::start_atom )//,
          //result<void>( caf::error )
        >;

typed_tester::behavior_type testDistribSystem(typed_tester::pointer self, typed_simple_task_runner<int, std::string> replier )
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
    after( std::chrono::microseconds( 200 ) ) >> [=]()
    {
      self->send(self, lfge::core::start_atom_v);
    }
  };
}

typed_tester::behavior_type testSAADistribSystem(typed_tester::pointer self, typed_split_and_agg_task_task_runner<std::size_t, std::size_t> replier )
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
      std::size_t inp = 100000;
      self->request( replier, caf::infinite, inp ).then(
        [=](std::size_t reply)
        {
          aout(self) << reply << std::endl;
        }
      );
    },
    after( std::chrono::seconds( 1000 ) ) >> [=]()
    {
      self->send(self, lfge::core::start_atom_v);
    }
  };
}



//Blocking test
/*sum
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
    std::stringstream ss;sum
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


CAF_BEGIN_TYPE_ID_BLOCK(custom_types_1, first_custom_type_id+100)

  CAF_ADD_TYPE_ID(custom_types_1, (std::pair<size_t, size_t>) )

CAF_END_TYPE_ID_BLOCK(custom_types_1)

// template <class Inspector>
// bool inspect(Inspector& f, std::pair<std::size_t, std::size_t>& x) {
//   return f.object(x).fields(f.field("x", x.first),
//                             f.field("y", x.second));
// }


void caf_main(actor_system& system, const caf::actor_system_config& cfg) {

  //lfge::core::logger::init(system);
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

  //auto staskdist = system.spawn< simple_distributor<int, std::string> >( "test", regservice );
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);
  //system.spawn(testDistribSystem, staskdist);

  using saa_distrib_type = split_and_aggregate_distributor< std::size_t, std::pair<std::size_t, std::size_t>, std::size_t, std::size_t >;
  auto splitterFunction = []( std::size_t till ) -> std::vector<std::pair<std::size_t, std::size_t>>
  {
    std::cout << " splitting " << std::endl;
    std::vector<std::pair<std::size_t, std::size_t>> retVec;
    for( std::size_t i = 0; i < till; ++i )
    {
      retVec.push_back( std::make_pair(i,i+1) );
    }
    std::cout << " splitted_size is " << retVec.size() << std::endl;
    return retVec;
  };

  auto aggregationFunction = []( const std::vector<std::size_t>& res )
  {
    return std::accumulate(res.begin(), res.end(), 0);
  };
  auto saa_distrib = system.spawn< saa_distrib_type >( "saa_test", regservice, splitterFunction, aggregationFunction );
  system.spawn( testSAADistribSystem, saa_distrib );

}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, id_block::custom_types_1, io::middleman)