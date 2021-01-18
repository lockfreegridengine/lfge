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
        static const char* log_group_name;

        static caf::expected<caf::group> getGroup();
        static void init( caf::actor_system& system );
        static void log( const loglevel& level, const std::string& message );
        static void stop();
    };

}

template <class Inspector>
bool inspect(Inspector& f, lfge::core::loglevel& x) {
    auto get = [&x] { return static_cast<uint8_t>(x); };
    auto set = [&x](uint8_t val) {
        x = static_cast<lfge::core::loglevel>(val);
        return true;
    };
    return f.apply(get, set);
};