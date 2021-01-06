#pragma once
#include "caf/all.hpp"
#include "core/core_atoms.hpp"

namespace lfge::core {

    using typed_timeout_actor = caf::typed_actor< caf::result<void>( timeout_atom ) >;

}