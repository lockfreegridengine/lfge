#pragma once
#include <string>
#include <caf/all.hpp>

namespace lfge::core
{

    enum class loglevel : uint8_t
    {
        comm = 0,
        debug,
        warning,
        error
    };

    class logger
    {
        public:

        static caf::expected<caf::group> getGroup();
        static void init( caf::actor_system& system );
        static void log( const loglevel& level, const std::string& message );
    };

}