#pragma once
#include "caf/all.hpp"
#include "core/core_atoms.hpp"

namespace lfge::resource_manager
{

    using ServiceName = std::string;
    using ServiceId = std::string;

    using typed_service_manager = caf::typed_event_based_actor<
                                    caf::result<void>( lfge::core::add_id_to_service, ServiceId, caf::actor),
                                    caf::result<void>( lfge::core::add_id_to_service, ServiceId, caf::actor_addr),
                                    caf::result<void>( lfge::core::remove_actor_for_service, ServiceId id ),
                                    caf::result< std::string, caf::actor_addr >( lfge::core::find_service )
                                    >;


    using typed_registering_client = caf::typed_actor<
        caf::result<void>( lfge::core::deregister_atom ),
        caf::result<void>( lfge::core::new_id_atom, std::string),
        caf::result<lfge::core::heartbeat_reply_atom, std::size_t>(lfge::core::heartbeat_atom, std::size_t)
    >;


    using typed_registration_actor = caf::typed_actor<
                                                caf::result<lfge::core::new_id_atom, std::string>(lfge::core::register_atom, ServiceName),
                                                caf::result<void>(lfge::core::register_atom, ServiceName, caf::actor_addr),
                                                caf::result<void>( lfge::core::register_atom, ServiceName, ServiceId),
                                                caf::result<void>( lfge::core::unregister_atom, ServiceName, ServiceId),
                                                caf::result<void>( lfge::core::unregister_atom, ServiceId ),
                                                caf::result<void>( lfge::core::remove_atom, ServiceName ),
                                                caf::result< std::string, caf::actor_addr >( lfge::core::find_service, ServiceName ),
                                                caf::result<void>( const caf::error ) 
                                                >;

}