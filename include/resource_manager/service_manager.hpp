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

    struct service_state
    {
        std::vector< std::pair<ServiceId, caf::actor> > idsAndActors;
        std::size_t current;
    };

    using typed_service_manager = caf::typed_event_based_actor<
                                    caf::result<void>( lfge::core::add_id_to_service, ServiceId, const caf::actor),
                                    caf::result<void>( lfge::core::remove_actor_for_service, ServiceId id ),
                                    caf::result< std::string, caf::actor_addr >( lfge::core::find_service )
                                    >;

    using typed_service_manager_hdl = typed_service_manager::actor_hdl;

    class service_manager : public typed_service_manager
    {
        ServiceName serviceName;
        caf::actor_addr creator;

        service_state state;
        
        public:
        service_manager( caf::actor_config& config, const ServiceName &serviceName, const caf::actor_addr creator );

        const ServiceName& getServiceName() const;

        behavior_type make_behavior() override;

        void clear();
    };
}