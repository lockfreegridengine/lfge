#include "ServiceManager.hpp"
#include "RegisteringService.hpp"

#include "core/core_atoms.hpp"

#include <algorithm>

using namespace lfge::resource_manager;

ServiceManager::ServiceManager( caf::actor_config& config, const ServiceName &serviceName, const caf::actor_addr creator ) 
            : typed_service_manager(config), serviceName(serviceName), creator(creator)
{
}

const ServiceName& ServiceManager::getServiceName() const
{
    return serviceName;
}

ServiceManager::behavior_type ServiceManager::make_behavior()
{
    this->set_exit_handler( [this](auto&& ptr, const caf::exit_msg&){
        auto actor = caf::actor_cast<RegisteringService::actor_hdl>(creator);
        ServiceName sName = getServiceName();
        this->send( actor, lfge::core::remove_atom_v, sName );
    } );

    return {
        [this]( lfge::core::add_id_to_service, ServiceId id, caf::actor actor)
        {
            this->state.idsAndActors.emplace_back( std::make_pair(id, actor) );
        },
        [this]( lfge::core::remove_actor_for_service, ServiceId id )
        {
            this->state.idsAndActors.erase( std::remove_if( this->state.idsAndActors.begin(), this->state.idsAndActors.end(), [id]( auto& p ){ return p.first == id; } ) );
            if(this->state.idsAndActors.empty())
            {
                auto actor = caf::actor_cast<RegisteringService::actor_hdl>(creator);
                ServiceName sName = getServiceName();
                this->send( actor, lfge::core::remove_atom_v, sName );
                quit();
            }
        },
        [this]( lfge::core::find_service ) -> caf::result< ServiceId, caf::actor_addr >
        {
            if( this->state.idsAndActors.empty() )
            {
                aout( this ) << "Service empty" << std::endl;
                return caf::sec::runtime_error;
            }
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
        anon_send( idAndActor.second, lfge::core::deregister_atom_v );
    }
}