#pragma once

#include "ServiceManager.hpp"
#include <random>

namespace lfge::resource_manager
{
    
    class RegisteringService;
    class ServiceRegistry
    {
        public:

        ServiceRegistry();

        void register_service( caf::event_based_actor& registeringService, const ServiceName &name, const ServiceId& id, caf::actor& actorToCall );

        void unregister_service( caf::event_based_actor& registeringService, const ServiceId& id );

        void unregister_service( caf::event_based_actor& registeringService,  const ServiceName &name, const ServiceId& id );

        bool contains( const ServiceId& id) const;

        private:
        void clear(caf::event_based_actor& registeringService);

        std::unordered_set<ServiceId> serviceIds;
        std::unordered_map< ServiceName, caf::actor > registeredServiceActors;
        friend class RegisteringService;
    };

    class RegisteringService : public caf::stateful_actor< ServiceRegistry >
    {
        private:
        std::random_device device;
        std::default_random_engine generator;
        std::string create_unique_id( const std::size_t size = 10 );

        public:

        static const std::string serviceName;

        RegisteringService(caf::actor_config& config);

        virtual caf::behavior make_behavior() override;
    };
}