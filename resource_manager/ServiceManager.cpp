#include "ServiceManager.hpp"
#include "core/core_atoms.hpp"

#include <algorithm>

using namespace lfge::resource_manager;

ServiceManager::ServiceManager( caf::actor_config& config, const std::string &serviceName, const caf::actor_addr creator ) 
            : caf::stateful_actor<ServiceState>(config), serviceName(serviceName), creator(creator)
{
}

const std::string& ServiceManager::getServiceName() const
{
    return serviceName;
}

caf::behavior ServiceManager::make_behavior()
{
    this->set_exit_handler( [this](auto&& ptr, const caf::exit_msg&){
        auto actor = caf::actor_cast<caf::actor>(creator);
        this->send( actor, lfge::core::remove_atom_v, getServiceName() );
    } );

    return {
        [this]( lfge::core::add_id_to_service, const ServiceId &id, const caf::actor &actor)
        {
            this->state.idsAndActors.emplace_back( std::make_pair(id, actor) );
        },
        [this]( lfge::core::remove_actor_for_service, const ServiceId& id )
        {
            this->state.idsAndActors.erase( std::remove_if( this->state.idsAndActors.begin(), this->state.idsAndActors.end(), [id]( auto& p ){ return p.first == id; } ) );
            if(this->state.idsAndActors.empty())
            {
                quit();
            }
        },
        [this]( lfge::core::find_service )
        {
            auto front = this->state.idsAndActors.front();
            this->state.idsAndActors.pop_front();
            this->state.idsAndActors.push_back(front);
            return caf::make_result( front.first, front.second.address() );
        }
    };
}

void ServiceManager::clear()
{
    for( auto idAndActor : this->state.idsAndActors )
    {
        this->send( idAndActor.second, lfge::core::deregister_atom_v );
    }
}