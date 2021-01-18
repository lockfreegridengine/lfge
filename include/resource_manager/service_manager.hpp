#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <list>

#include "resource_manager/actor_defs.hpp"

namespace lfge::resource_manager
{
    using typed_service_manager_hdl = typed_service_manager::actor_hdl;

    struct service_state
    {
        std::vector< std::pair<ServiceId, caf::actor> > idsAndActors;
        std::size_t current;
    };

    class service_manager : public typed_service_manager
    {
        ServiceName serviceName;
        typed_registration_actor creator;

        service_state state;
        
        public:
        service_manager( caf::actor_config& config, const ServiceName &serviceName, typed_registration_actor creator );

        const ServiceName& getServiceName() const;

        behavior_type make_behavior() override;

        void clear();
    };
}