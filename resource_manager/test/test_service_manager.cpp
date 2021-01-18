#define CAF_SUITE rm_tests

#include "caf/test/dsl.hpp"
#include "core/logger.hpp"
#include "core/core_atoms.hpp"
#include <chrono>

#include "resource_manager/actor_defs.hpp"
#include "resource_manager/service_manager.hpp"

using namespace caf;
using namespace lfge::core;
using namespace lfge::resource_manager;

typed_registration_actor::behavior_type mock_reg_system( typed_registration_actor::pointer self )
{
    return {
        [](lfge::core::register_atom, ServiceName)-> caf::result<lfge::core::new_id_atom, std::string>
        {  return make_result( new_id_atom_v, std::string("test") ); },
        [](lfge::core::register_atom, ServiceName, caf::actor_addr){},
        []( lfge::core::register_atom, ServiceName, ServiceId){},
        []( lfge::core::unregister_atom, ServiceName, ServiceId){},
        []( lfge::core::unregister_atom, ServiceId ){},
        []( lfge::core::remove_atom, ServiceName ){},
        []( lfge::core::find_service, ServiceName ) -> caf::result< std::string, caf::actor_addr >
        { CAF_FAIL( "Should not come here" ); },
        []( const caf::error ) {}
    };
}

struct service_manager_fixture : test_coordinator_fixture<>
{

    typed_service_manager_hdl serviceManager;
    typed_registration_actor regSystem;
    ServiceName sname = "test";
    service_manager_fixture()
    {
        auto regSystem = sys.spawn(mock_reg_system);
        serviceManager = sys.spawn<service_manager>( sname, regSystem );
        run();
    }
};

CAF_TEST_FIXTURE_SCOPE( service_manager_tests, service_manager_fixture )

CAF_TEST(service_manager_add_find_remove_service)
{
    // Adding a service to service manager and checking if the message is in the box
    self->send( serviceManager, lfge::core::add_id_to_service_v, ServiceId("ABC"), self.address() );
    expect( (lfge::core::add_id_to_service, ServiceId, caf::actor_addr), from( self ).to(serviceManager).with( lfge::core::add_id_to_service_v, std::string("ABC"), self.address()  ) );
    
    // Finding an actor providing the service 
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );
    // Redoing the same find
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );

    // Adding another actor to the service manager
    caf::scoped_actor act2{sys};
    self->send( serviceManager, lfge::core::add_id_to_service_v, ServiceId("BCD"), act2.address() );
    expect( (lfge::core::add_id_to_service, ServiceId, caf::actor_addr), from( self ).to(serviceManager).with( lfge::core::add_id_to_service_v, std::string("BCD"), act2.address()  ) );
    
    
    // Finding the service will send two added actors in round robin fashion (check multiple times)
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );
     self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("ABC"), self.address()) );


    // Removing a ABC from the service manager
    self->send( serviceManager, remove_actor_for_service_v, std::string("ABC") );
    expect( (lfge::core::remove_actor_for_service, ServiceId), from(self).to(serviceManager).with(_,_) );

    // Check there is only one actor proving service found
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (std::string, caf::actor_addr), from(serviceManager).to(self).with(std::string("BCD"), act2.address()) );

    //removing all the services
    self->send( serviceManager, remove_actor_for_service_v, std::string("BCD") );
    expect( (lfge::core::remove_actor_for_service, ServiceId), from(self).to(serviceManager).with(_,_) );
    run();
    
}

CAF_TEST(finding_service_which_does_not_exist)
{
    self->send( serviceManager, lfge::core::find_service_v );
    expect( (lfge::core::find_service), from(self).to(serviceManager).with(lfge::core::find_service_v) );
    expect( (caf::error), from(serviceManager).to(self).with(caf::sec::runtime_error) );
}

CAF_TEST_FIXTURE_SCOPE_END()