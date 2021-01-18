#include "resource_manager/registering_service.hpp"
#include <algorithm>
#include "core/heartbeat.hpp"
#include "core/logger.hpp"


using namespace lfge::resource_manager;
using namespace lfge::core;

const std::string registering_service::reg_srv_name = "registering_service";


registering_service::registering_service(caf::actor_config& config) : subscriber_typed_registration_actor(config), device(), generator(device())
{
    logger::log( loglevel::comm, "Creating a registration service" );
}

void registering_service::register_service( const ServiceName &name, const ServiceId& id, caf::actor& actorToCall )
{
    logger::log( loglevel::comm, "Registering " + id + " to service " + name );
    auto ite = registeredServiceActors.find(name);
    if( ite == registeredServiceActors.end() )
    {
        // Service does not exists
        auto newService = spawn<service_manager>( name, caf::actor_cast<typed_registration_actor>(address()) );

        registeredServiceActors.insert( std::make_pair( name, newService ) );
        send( newService, lfge::core::add_id_to_service_v, id, actorToCall );
    }
    else
    {
        // Service already exists
        send( ite->second, lfge::core::add_id_to_service_v, id, actorToCall );
    }
    
    serviceIds.insert(id);
}

void registering_service::unregister_service( const ServiceId& id )
{
    logger::log( loglevel::comm, "Unregistering" + id + " from all services" );
    for( auto& service : registeredServiceActors )
    {
        send( service.second, lfge::core::remove_actor_for_service_v, id );
    }
}

void registering_service::unregister_service(  const ServiceName &name, const ServiceId& id )
{
    logger::log( loglevel::comm, "Unregistering " + name + " from " + name + " services" );
    auto serviceIte = registeredServiceActors.find(name);
    if( serviceIte != registeredServiceActors.end() )
    {
        send( serviceIte->second, lfge::core::remove_actor_for_service_v, id );
    }
}

bool registering_service::contains( const ServiceId& id) const
{
    return serviceIds.count( id ) > 0;
}

void registering_service::clear(  )
{
    logger::log( loglevel::comm, " Clearing all registered services" );
    for( auto & p : registeredServiceActors )
    {
        registeredServiceActors.clear();
        send_exit( p.second, caf::sec::none );
    }
    registeredServiceActors.clear();
    serviceIds.clear();
}

void registering_service::spawn_hb_monitor(const ServiceName &serviceName, const ServiceId& id, caf::actor& actorToCall)
{
    // TODO : Set timeout and pulse duration in config
    this->system().spawn<lfge::core::heartbeat_sender>( actorToCall, 100, 10, 3, id, [this, serviceName, id](auto&&){
        logger::log( loglevel::comm, "Heartbeat timeout for " + id + " registered to service " + serviceName );
        this->send(this,lfge::core::unregister_atom_v, serviceName, id );
    } );
}

std::string registering_service::create_unique_id( const std::size_t size )
{
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), [this](){ return 'A' + generator()%25; });

    return contains(str) ? create_unique_id() : str;
}



registering_service::behavior_type registering_service::make_behavior()
{
    auto grp = this->system().groups().get_local( reg_srv_name );
    this->join( grp );

    this->set_exit_handler( [this]( auto&& ptr, const caf::exit_msg& ){
        clear();
    } );
    
    return {
        [this]( lfge::core::register_atom, ServiceName serviceName )
        {
            std::string id = create_unique_id();
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );

            register_service( serviceName, id, actor);

            spawn_hb_monitor(serviceName, id, actor);

            return caf::make_result( lfge::core::new_id_atom_v, id);
        },
        [this](lfge::core::register_atom, ServiceName serviceName, caf::actor_addr addr)
        {
            std::string id = create_unique_id();
            caf::actor actor = caf::actor_cast<caf::actor>( addr );

            register_service( serviceName, id, actor);

            spawn_hb_monitor(serviceName, id, actor);

            anon_send( actor, lfge::core::new_id_atom_v, id );
        },
        [this]( lfge::core::register_atom, ServiceName serviceName, ServiceId id )
        {
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );
            
            register_service( serviceName, id, actor);

            spawn_hb_monitor(serviceName, id, actor);

        },
        [this]( lfge::core::unregister_atom, ServiceName serviceName, ServiceId id )
        {
            unregister_service( serviceName, id );
        }, 
        [this]( lfge::core::unregister_atom, ServiceId id )
        {
            unregister_service( id );
        },
        [this]( lfge::core::remove_atom, ServiceName name )
        {
            logger::log( loglevel::warning, "Removing service " + name );
            auto ite = registeredServiceActors.find( name );
            if(ite !=  registeredServiceActors.end())
            {
                registeredServiceActors.erase(name);
                send_exit( ite->second, caf::sec::none );
            }
        },
        [this]( lfge::core::find_service, ServiceName serviceName ) -> caf::result< std::string, caf::actor_addr >
        {
            auto ite = registeredServiceActors.find(serviceName);
            if( ite == registeredServiceActors.end() )
            {
                logger::log( loglevel::warning, "service " + serviceName + " was requested but not found" );
                return lfge_errors::service_not_found;
            }
            return this->delegate( ite->second, lfge::core::find_service_v);
        },
        [this]( const caf::error& e )
        {
            logger::log( loglevel::error, " Registering service got an error " + to_string(e) );
        }
    };
}