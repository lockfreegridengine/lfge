#include "core/logger.hpp"
#include "core/core_atoms.hpp"

using namespace lfge::core;

caf::actor_system *act_system = nullptr;

void logger::init( caf::actor_system& system )
{
    act_system = &system;
}

void logger::log( const loglevel &level, const std::string& message )
{
    if( act_system )
    {
        static auto grp = act_system->groups().get( "local", "log" );
        caf::scoped_actor self {*act_system};
        self->send(*grp, level, message);
    }
}