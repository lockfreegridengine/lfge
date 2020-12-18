#include "RegisteringService.hpp"
#include <algorithm>
#include "core/Heartbeat.hpp"


using namespace lfge::resource_manager;

const std::string RegisteringService::serviceName = "registering_service";


RegisteringService::RegisteringService(caf::actor_config& config) : subscriber_typed_registration_actor(config), device(), generator(device())
{
}

void RegisteringService::register_service( const ServiceName &name, const ServiceId& id, caf::actor& actorToCall )
{
    auto ite = registeredServiceActors.find(name);
    if( ite == registeredServiceActors.end() )
    {
        // Service does not exists
        auto newService = spawn<ServiceManager>( name, address() );

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

void RegisteringService::unregister_service( const ServiceId& id )
{
    for( auto& service : registeredServiceActors )
    {
        send( service.second, lfge::core::remove_actor_for_service_v, id );
    }
}

void RegisteringService::unregister_service(  const ServiceName &name, const ServiceId& id )
{
    auto serviceIte = registeredServiceActors.find(name);
    if( serviceIte != registeredServiceActors.end() )
    {
        send( serviceIte->second, lfge::core::remove_actor_for_service_v, id );
    }
}

bool RegisteringService::contains( const ServiceId& id) const
{
    return serviceIds.count( id ) > 0;
}

void RegisteringService::clear(  )
{

    for( auto & p : registeredServiceActors )
    {
        registeredServiceActors.clear();
        send_exit( p.second, caf::sec::none );
    }
    registeredServiceActors.clear();
    serviceIds.clear();
}

std::string RegisteringService::create_unique_id( const std::size_t size )
{
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), [this](){ return 'A' + generator()%25; });

    return contains(str) ? create_unique_id() : str;
}

RegisteringService::behavior_type RegisteringService::make_behavior()
{
    auto grp = this->system().groups().get( "local", serviceName );
    if(grp)
    {
        this->join( *grp );
    }
    else
    {
        std::cerr << "Cannot join the group registering system";
    }

    this->set_exit_handler( [this]( auto&& ptr, const caf::exit_msg& ){
        clear();
    } );
    
    return {
        [this]( lfge::core::register_atom, ServiceName serviceName )
        {
            caf::aout(this) << "Registering " << std::endl;
            std::string id = create_unique_id();
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );

            register_service( serviceName, id, actor);

            // TODO : Set timeout and pulse duration in config
            this->system().spawn<lfge::core::HeartbeatSender>(actor, 100, 10, 3, "", [this, serviceName, id](auto&&){
                aout(this) << "unregistering id " << id << std::endl;
                this->send(this,lfge::core::unregister_atom_v, serviceName, id );
            } );

            return caf::make_result( lfge::core::new_id_atom_v, id);
        },
        [this]( lfge::core::register_atom, ServiceName serviceName, ServiceId id )
        {
            caf::aout(this) << "Registering " << std::endl;
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );
            register_service( serviceName, id, actor);

            // TODO : Set timeout and pulse duration in config
            this->system().spawn<lfge::core::HeartbeatSender>(actor, 100, 10, 3, "", [this, serviceName, id](auto&&){
                aout(this) << "unregistering id " << id << std::endl;
                this->send(this,lfge::core::unregister_atom_v, serviceName, id );
            } );

        },
        [this]( lfge::core::unregister_atom, ServiceName serviceName, ServiceId id )
        {
            caf::aout(this) << "Unregistering " << std::endl;
            unregister_service( serviceName, id );
        }, 
        [this]( lfge::core::unregister_atom, ServiceId id )
        {
            caf::aout(this) << "Unregistering " << std::endl;
            unregister_service( id );
        },
        [this]( lfge::core::remove_atom, ServiceName name )
        {
            caf::aout(this) << "Removing from " << std::endl;
            registeredServiceActors.erase(name);
        },
        [this]( lfge::core::find_service, ServiceName serviceName ) -> caf::result< std::string, caf::actor_addr >
        {
            auto ite = registeredServiceActors.find(serviceName);
            if( ite == registeredServiceActors.end() )
            {
                aout( this ) << "Service not found" << std::endl;
                return caf::sec::unexpected_message;
            }
            return this->delegate( ite->second, lfge::core::find_service_v);
        },
        [this]( const caf::error )
        {
            caf::aout(this) << "Error code received on registering system " << std::endl;
        }
    };
}