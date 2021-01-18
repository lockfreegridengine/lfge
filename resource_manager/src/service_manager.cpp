#include "resource_manager/service_manager.hpp"
#include "resource_manager/registering_service.hpp"

#include "core/core_atoms.hpp"
#include "core/logger.hpp"

#include <algorithm>

using namespace lfge::resource_manager;
using namespace lfge::core;

service_manager::service_manager( caf::actor_config& config, const ServiceName &serviceName, typed_registration_actor creator ) 
            : typed_service_manager(config), serviceName(serviceName), creator(creator)
{
    logger::log( loglevel::comm, "Creating a service named " + serviceName );
    this->state.current = 0;
}

const ServiceName& service_manager::getServiceName() const
{
    return serviceName;
}

service_manager::behavior_type service_manager::make_behavior()
{
    this->set_exit_handler( [this](auto&& ptr, const caf::exit_msg&){
        logger::log( loglevel::comm, "Exiting a service named " + getServiceName() );
        this->send( creator, lfge::core::remove_atom_v, getServiceName() );
    } );

    return {
        [this]( lfge::core::add_id_to_service, ServiceId id, caf::actor actor)
        {
            logger::log( loglevel::comm, "Adding " + id + " to service " + getServiceName() );
            this->state.idsAndActors.emplace_back( std::make_pair(id, actor) );
        },
        [this]( lfge::core::add_id_to_service, ServiceId id, caf::actor_addr actorAddr)
        {
            logger::log( loglevel::comm, "Adding " + id + " to service " + getServiceName() );
            caf::actor act = caf::actor_cast<caf::actor>(actorAddr);
            this->state.idsAndActors.emplace_back( std::make_pair(id, act) );
        },
        [this]( lfge::core::remove_actor_for_service, ServiceId id )
        {
            logger::log( loglevel::comm, "Removing " + id + " from service " + getServiceName() );
            this->state.idsAndActors.erase( std::remove_if( this->state.idsAndActors.begin(), this->state.idsAndActors.end(), [id]( auto& p ){ return p.first == id; } ) );
            if(this->state.idsAndActors.empty())
            {
                this->send( creator, lfge::core::remove_atom_v, getServiceName() );
            }
        },
        [this]( lfge::core::find_service ) -> caf::result< ServiceId, caf::actor_addr >
        {
            if( this->state.idsAndActors.empty() )
            {
                logger::log( loglevel::error, "Service " + getServiceName() + " is empty" );
                return caf::sec::runtime_error;
            }
            if( this->state.current >= this->state.idsAndActors.size() )
            {
                this->state.current = 0;
            }
            auto front = this->state.idsAndActors[ this->state.current ];
            ++this->state.current;

            logger::log( loglevel::comm, "Requester for " + getServiceName() + " got " + front.first );
            return caf::make_result( front.first, front.second.address() );
        }
    };
}

void service_manager::clear()
{
    logger::log( loglevel::comm, "Clearing all services for " + getServiceName() );
    for( auto idAndActor : this->state.idsAndActors )
    {
        anon_send( idAndActor.second, lfge::core::deregister_atom_v );
    }
}