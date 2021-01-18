#include "core/logger.hpp"
#include "core/core_atoms.hpp"
#include "core/lfge_errors.hpp"

using namespace lfge::core;

caf::actor_system *act_system = nullptr;
const char *logger::log_group_name = "log";

caf::expected<caf::group> logger::getGroup()
{
    return act_system ? act_system->groups().get("local", log_group_name) : lfge_errors::logger_not_initialized;
}

void logger::init( caf::actor_system& system )
{
    act_system = &system;
}

void logger::log( const loglevel &level, const std::string& message )
{
    if( act_system )
    {
        static caf::group grp = act_system->groups().get_local( log_group_name );
        caf::scoped_actor self {*act_system};
        self->send(grp, level, message);
    }
}

void logger::stop()
{
    act_system = nullptr;
}