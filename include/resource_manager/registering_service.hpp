#pragma once

#include "service_manager.hpp"
#include <random>

namespace lfge::resource_manager
{
    class registering_service;

    using subscriber_typed_registration_actor = caf::extend<typed_registration_actor::base>::with<caf::mixin::subscriber>;

    class registering_service : public subscriber_typed_registration_actor
    {
        private:
        std::random_device device;
        std::default_random_engine generator;
        std::string create_unique_id( const std::size_t size = 10 );

        void register_service( const ServiceName &name, const ServiceId& id, caf::actor& actorToCall );

        void unregister_service( const ServiceId& id );

        void unregister_service( const ServiceName &name, const ServiceId& id );

        bool contains( const ServiceId& id) const;

        void clear();

        void spawn_hb_monitor(const ServiceName &serviceName, const ServiceId& id, caf::actor& actorToCall);

        std::unordered_set<ServiceId> serviceIds;

        std::unordered_map< ServiceName, typed_service_manager_hdl > registeredServiceActors;

        public:

        static const std::string reg_srv_name;

        registering_service(caf::actor_config& config);

        virtual behavior_type make_behavior() override;
    };
}