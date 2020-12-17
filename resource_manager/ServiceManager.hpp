#pragma once

#include "caf/all.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>
#include <list>

namespace lfge::resource_manager
{
    using ServiceName = std::string;
    using ServiceId = std::string;

    struct ServiceState
    {
        std::list< std::pair<ServiceId, caf::actor> > idsAndActors;
    };

    class ServiceManager : public caf::stateful_actor<ServiceState>
    {
        ServiceName serviceName;
        caf::actor_addr creator;
        public:
        ServiceManager( caf::actor_config& config, const std::string &serviceName, const caf::actor_addr creator );

        const std::string& getServiceName() const;

        caf::behavior make_behavior() override;

        void clear();
    };
}