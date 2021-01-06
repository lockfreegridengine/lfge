#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "core/core_atoms.hpp"
#include "resource_manager/registering_service.hpp"
#include "core/heartbeat.hpp"
#include "resource_manager/treatment.hpp"
#include "core/core_typed_actors.hpp"

using namespace caf;
using namespace lfge::core;
using namespace lfge::resource_manager;


behavior registering_client( caf::stateful_actor<std::string>* self )
{
    auto grp = self->system().groups().get( "remote", std::string("registering_service") + "@localhost:4555" );
    auto hb_replier = self->spawn<lfge::core::heartbeat_receiver> ("", nullptr, 2000, [=](auto&&){ 
        aout(self) << "Got no hb for 2000 ms" << std::endl;
        self->send_exit(self, caf::sec::request_timeout); 
        } );

    //self.join(lfge::resource_manager::registering_service::serviceName + "@localhost:4555" );
    self->send( *grp, lfge::core::register_atom_v, "test1" );
    return {
        [=]( lfge::core::deregister_atom )
        {
            aout(self) << "deregistering client" << std::endl;
        },
        [=]( lfge::core::new_id_atom, std::string id )
        {
            aout(self) << "got new id " << id << std::endl;

            //self->delayed_send(*grp, std::chrono::seconds(20), lfge::core::unregister_atom_v, id);
        },
        [=]( lfge::core::string_data_atom, std::string data )
        {
            aout(self) << "Got message " << data << std::endl;
        },
        [=]( lfge::core::heartbeat_atom, std::size_t index )
        {
            return self->delegate(hb_replier, lfge::core::heartbeat_atom_v, index);
        },
        [=]( caf::error& e)
        {
            aout(self) << "Got error " << to_string(e) << std::endl;
        }
    };
}

using testrepier = simple_int_replyer::extend_with<typed_registering_client>::extend_with<lfge::core::typed_timeout_actor>;
static int testDistrib = 0;
testrepier::behavior_type test_simple_distribution( testrepier::stateful_pointer<int> self, const std::string& serviceName )
{

    auto hb_replier = self->spawn<lfge::core::heartbeat_receiver> ("", nullptr, 2000, [=](auto&&){ 
        aout(self) << "Got no hb for 2000 ms" << std::endl;
        self->send_exit(self, caf::sec::request_timeout); 
        } );
    self->state = 0;
    return {

        [=]( std::string data ) -> caf::result <int>
        {
            //aout(self) << "Got " << data << std::endl;
            return ++self->state;
        },
        [=]( lfge::core::deregister_atom )
        {
            aout(self) << "deregistering client" << std::endl;
        },
        [=]( lfge::core::new_id_atom, std::string id )
        {
            aout(self) << "got new id " << id << std::endl;

            //self->delayed_send(*grp, std::chrono::seconds(20), lfge::core::unregister_atom_v, id);
        },
        [=]( lfge::core::heartbeat_atom, std::size_t index )
        {
            return self->delegate(hb_replier, lfge::core::heartbeat_atom_v, index);
        },
        [=]( lfge::core::timeout_atom )
        {
             aout(self) << self->state << std::endl;
             self->state = 0;
        }
    };
}

behavior timeout_sender( caf::event_based_actor* self, caf::actor_addr act )
{
    return {
        after(std::chrono::seconds(1)) >> [=]()
        {
            self->send( caf::actor_cast<caf::actor>(act), lfge::core::timeout_atom_v );
        }
    };
}

behavior hbreplier( event_based_actor* self )
{
    return {

        [=]( lfge::core::heartbeat_atom, const std::size_t& messageIndex )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(400) );
            return make_result( lfge::core::heartbeat_reply_atom_v, messageIndex );
        }
    };
} 

void caf_main(actor_system& system, const caf::actor_system_config& cfg) {
    //system.spawn(registering_client);
    auto distrib = system.spawn(test_simple_distribution, "test");

    auto grp = system.groups().get( "remote", std::string("registering_service") + "@localhost:4555" );
    if( grp == nullptr )
    {
        std::cout << " ERROR GROUP NOT FOUND" << std::endl;
    }
    else
    {
        caf::scoped_actor act{system};
        act->send( *grp, lfge::core::register_atom_v, "test", distrib.address() );
        std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    }

    system.spawn( timeout_sender, distrib.address() );
    // auto hb_replier = system.spawn<lfge::core::heartbeat_receiver>("toto", 
    //     []( lfge::core::heartbeat_receiver&, const std::size_t & index ) { std::cout << "HB " << index << "\n"; }, 
    //     10000,
    //     [](lfge::core::heartbeat_receiver&) {  std::cout << "HB RECIEVER TIMEOUT" << std::endl;  }
    // );

    // system.spawn<lfge::core::heartbeat_sender>( hb_replier , 1000, 500, 3, "MyActor");
}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, io::middleman)