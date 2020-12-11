#pragma once
#include <caf/all.hpp>

CAF_BEGIN_TYPE_ID_BLOCK(lfge_core, first_custom_type_id)

CAF_ADD_ATOM(lfge_core, lfge::core, heartbeat_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, heartbeat_reply_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, timeout_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, start_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, current_index_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, read_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, write_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, contains_atom)

CAF_END_TYPE_ID_BLOCK(lfge_core)