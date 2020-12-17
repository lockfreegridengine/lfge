#include "RegisteringService.hpp"
#include <algorithm>
#include "core/core_atoms.hpp"
#include "core/Heartbeat.hpp"


using namespace lfge::resource_manager;

const std::string RegisteringService::serviceName = "registering_service";

ServiceRegistry::ServiceRegistry()
{
}

void ServiceRegistry::register_service( caf::event_based_actor& registeringService, const ServiceName &name, const ServiceId& id, caf::actor& actorToCall )
{
    auto ite = registeredServiceActors.find(name);
    if( ite == registeredServiceActors.end() )
    {
        // Service does not exists
        auto newService = registeringService.spawn<ServiceManager>( name, registeringService.address() );

        registeredServiceActors.insert( std::make_pair( name, newService ) );
        registeringService.send( newService, lfge::core::add_id_to_service_v, id, actorToCall );
    }
    else
    {
        // Service already exists
        registeringService.send( ite->second, lfge::core::add_id_to_service_v, id, actorToCall );
    }
    
    serviceIds.insert(id);
}

void ServiceRegistry::unregister_service( caf::event_based_actor& registeringService, const ServiceId& id )
{
    for( auto& service : registeredServiceActors )
    {
        registeringService.send( service.second, lfge::core::remove_actor_for_service_v, id );
    }
}

void ServiceRegistry::unregister_service( caf::event_based_actor& registeringService,  const ServiceName &name, const ServiceId& id )
{
    auto serviceIte = registeredServiceActors.find(name);
    if( serviceIte != registeredServiceActors.end() )
    {
        registeringService.send( serviceIte->second, lfge::core::remove_actor_for_service_v, id );
    }
}

bool ServiceRegistry::contains( const ServiceId& id) const
{
    return serviceIds.count( id ) > 0;
}

void ServiceRegistry::clear(caf::event_based_actor& registeringService)
{

    for( auto & p : registeredServiceActors )
    {
        registeredServiceActors.clear();
        registeringService.send_exit( p.second, caf::sec::none );
    }
    registeredServiceActors.clear();
    serviceIds.clear();
}

RegisteringService::RegisteringService(caf::actor_config& config) : caf::stateful_actor< ServiceRegistry >(config), device(), generator(device())
{
}

std::string RegisteringService::create_unique_id( const std::size_t size )
{
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), [this](){ return 'A' + generator()%25; });

    return this->state.contains(str) ? create_unique_id() : str;
}

caf::behavior RegisteringService::make_behavior()
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
        this->state.clear(*this);
    } );
    
    return {
        [this]( lfge::core::register_atom, const ServiceName& serviceName )
        {
            caf::aout(this) << "Registering " << std::endl;
            std::string id = create_unique_id();
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );

            this->state.register_service( *this, serviceName, id, actor);

            // TODO : Set timeout and pulse duration in config
            this->system().spawn<lfge::core::HeartbeatSender>(actor, 100, 10, 3, "", [this, serviceName, id](auto&&){
                aout(this) << "unregistering id " << id << std::endl;
                this->send(this,lfge::core::unregister_atom_v, serviceName, id );
            } );

            return caf::make_result( lfge::core::new_id_atom_v, id);
        },
        [this]( lfge::core::register_atom, const ServiceName& serviceName, const ServiceId& id )
        {
            caf::aout(this) << "Registering " << std::endl;
            caf::actor actor = caf::actor_cast<caf::actor>( this->current_sender() );
            this->state.register_service(*this, serviceName, id, actor);

            // TODO : Set timeout and pulse duration in config
            this->system().spawn<lfge::core::HeartbeatSender>(actor, 100, 10, 3, "", [this, serviceName, id](auto&&){
                aout(this) << "unregistering id " << id << std::endl;
                this->send(this,lfge::core::unregister_atom_v, serviceName, id );
            } );

        },
        [this]( lfge::core::unregister_atom, const ServiceName& serviceName, ServiceId id )
        {
            caf::aout(this) << "Unregistering " << std::endl;
            this->state.unregister_service( *this, serviceName, id );
        }, 
        [this]( lfge::core::unregister_atom, const ServiceId& id )
        {
            caf::aout(this) << "Unregistering " << std::endl;
            this->state.unregister_service( *this, id );
        },
        [this]( lfge::core::remove_atom, const ServiceName& name )
        {
            caf::aout(this) << "Removing from " << std::endl;
            this->state.registeredServiceActors.erase(name);
        },
        [this]( lfge::core::find_service, const ServiceName& serviceName )
        {
            return this->delegate(this->state.registeredServiceActors[serviceName], lfge::core::find_service_v);
        },
        [this]( caf::error& )
        {
            caf::aout(this) << "Error code received on registering system " << std::endl;
        }
    };
}