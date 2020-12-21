#pragma once
#include <string>
#include <caf/all.hpp>

namespace lfge::core
{

    enum class loglevel : uint8_t
    {
        eComm,
        eDebug,
        eWarning,
        eError
    };

    class logger
    {
        public:
        static void init( caf::actor_system& system );
        static void log( const loglevel& level, const std::string& message );
    };

}