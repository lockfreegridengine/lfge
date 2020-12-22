#include "service_manager.hpp"
#include "registering_service.hpp"

#include "core/core_atoms.hpp"
#include "core/logger.hpp"

#include <algorithm>

using namespace lfge::resource_manager;
using namespace lfge::core;

service_manager::service_manager( caf::actor_config& config, const ServiceName &serviceName, const caf::actor_addr creator ) 
            : typed_service_manager(config), serviceName(serviceName), creator(creator)
{
    logger::log( loglevel::comm, "Creating a service named " + serviceName );
}

const ServiceName& service_manager::getServiceName() const
{
    return serviceName;
}

service_manager::behavior_type service_manager::make_behavior()
{
    this->set_exit_handler( [this](auto&& ptr, const caf::exit_msg&){
        logger::log( loglevel::comm, "Exiting a service named " + getServiceName() );
        auto actor = caf::actor_cast<registering_service::actor_hdl>(creator);
        this->send( actor, lfge::core::remove_atom_v, getServiceName() );
    } );

    return {
        [this]( lfge::core::add_id_to_service, ServiceId id, caf::actor actor)
        {
            logger::log( loglevel::comm, "Adding " + id + " to service " + getServiceName() );
            this->state.idsAndActors.emplace_back( std::make_pair(id, actor) );
        },
        [this]( lfge::core::remove_actor_for_service, ServiceId id )
        {
            logger::log( loglevel::comm, "Removing " + id + " from service " + getServiceName() );
            this->state.idsAndActors.erase( std::remove_if( this->state.idsAndActors.begin(), this->state.idsAndActors.end(), [id]( auto& p ){ return p.first == id; } ) );
            if(this->state.idsAndActors.empty())
            {
                auto actor = caf::actor_cast<registering_service::actor_hdl>(creator);
                this->send( actor, lfge::core::remove_atom_v, getServiceName() );
                quit();
            }
        },
        [this]( lfge::core::find_service ) -> caf::result< ServiceId, caf::actor_addr >
        {
            if( this->state.idsAndActors.empty() )
            {
                logger::log( loglevel::error, "Service " + getServiceName() + " is empty" );
                return caf::sec::runtime_error;
            }
            auto front = this->state.idsAndActors.front();
            this->state.idsAndActors.pop_front();
            this->state.idsAndActors.push_back(front);

            logger::log( loglevel::error, "Requester for " + getServiceName() + " got " + front.first );
            return caf::make_result( front.first, front.second.address() );
        }
    };
}

void service_manager::clear()
{
    logger::log( loglevel::error, "Clearing all services for " + getServiceName() );
    for( auto idAndActor : this->state.idsAndActors )
    {
        anon_send( idAndActor.second, lfge::core::deregister_atom_v );
    }
}