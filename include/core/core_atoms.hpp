#pragma once
#include <caf/all.hpp>
#include "core/lfge_errors.hpp"
#include "core/logger.hpp"

CAF_BEGIN_TYPE_ID_BLOCK(lfge_core, first_custom_type_id)

CAF_ADD_TYPE_ID(lfge_core, (lfge_errors))
CAF_ADD_TYPE_ID(lfge_core, (lfge::core::loglevel))

// General atoms
CAF_ADD_ATOM(lfge_core, lfge::core, get_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, timeout_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, start_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, current_index_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, contains_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, remove_atom)

//Heartbeat atoms
CAF_ADD_ATOM(lfge_core, lfge::core, heartbeat_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, heartbeat_reply_atom)

// Data Container Atoms
CAF_ADD_ATOM(lfge_core, lfge::core, read_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, write_atom)

// Service atoms
CAF_ADD_ATOM(lfge_core, lfge::core, add_id_to_service)
CAF_ADD_ATOM(lfge_core, lfge::core, remove_actor_for_service)
CAF_ADD_ATOM(lfge_core, lfge::core, find_service)

// Communication Atoms
CAF_ADD_ATOM(lfge_core, lfge::core, register_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, unregister_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, deregister_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, new_id_atom)

// Data Atoms
CAF_ADD_ATOM(lfge_core, lfge::core, binary_data_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, string_data_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, serializable_data_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, proto_buf_data_atom)

CAF_END_TYPE_ID_BLOCK(lfge_core)

CAF_ERROR_CODE_ENUM(lfge_errors)