#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "core/core_atoms.hpp"
#include "../resource_manager/RegisteringService.hpp"
#include "core/Heartbeat.hpp"

using namespace caf;


behavior registering_client( caf::stateful_actor<std::string>* self )
{
    auto grp = self->system().groups().get( "remote", std::string("registering_service") + "@localhost:4555" );
    //self.join(lfge::resource_manager::RegisteringService::serviceName + "@localhost:4555" );
    self->send( *grp, lfge::core::register_atom_v, "test" );
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
            return make_result( lfge::core::heartbeat_reply_atom_v, index );
        },
        [=]( caf::error& e)
        {
            aout(self) << "Got error " << to_string(e) << std::endl;
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
    system.spawn(registering_client);

    // auto hb_replier = system.spawn<lfge::core::HeartbeatReceiver>("toto", 
    //     []( lfge::core::HeartbeatReceiver&, const std::size_t & index ) { std::cout << "HB " << index << "\n"; }, 
    //     10000,
    //     [](lfge::core::HeartbeatReceiver&) {  std::cout << "HB RECIEVER TIMEOUT" << std::endl;  }
    // );

    // system.spawn<lfge::core::HeartbeatSender>( hb_replier , 1000, 500, 3, "MyActor");
}

// creates a main function for us that calls our caf_main
CAF_MAIN( caf::id_block::lfge_core, io::middleman)