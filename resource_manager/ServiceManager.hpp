#pragma once

#include "caf/all.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <list>

#include "core/core_atoms.hpp"

namespace lfge::resource_manager
{
    using ServiceName = std::string;
    using ServiceId = std::string;

    struct ServiceState
    {
        std::list< std::pair<ServiceId, caf::actor> > idsAndActors;
    };

    using typed_service_manager = caf::typed_event_based_actor<
                                    caf::result<void>( lfge::core::add_id_to_service, ServiceId, const caf::actor),
                                    caf::result<void>( lfge::core::remove_actor_for_service, ServiceId id ),
                                    caf::result< std::string, caf::actor_addr >( lfge::core::find_service )
                                    >;

    using typed_service_manager_hdl = typed_service_manager::actor_hdl;

    class ServiceManager : public typed_service_manager
    {
        ServiceName serviceName;
        caf::actor_addr creator;

        ServiceState state;
        public:
        ServiceManager( caf::actor_config& config, const ServiceName &serviceName, const caf::actor_addr creator );

        const ServiceName& getServiceName() const;

        behavior_type make_behavior() override;

        void clear();
    };
}