#pragma once

#include "caf/all.hpp"
#include <string>

namespace lfge::resource_manager
{
    class RMConfig : public caf::actor_system_config
    {
        uint16_t registering_service_port = 0;
        uint16_t heartbeat_port = 0;
        public:
            RMConfig();
    };
}