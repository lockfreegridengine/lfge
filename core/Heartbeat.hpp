#pragma once

#include <chrono>
#include <cassert>
#include <exception>
#include "caf/all.hpp"


CAF_BEGIN_TYPE_ID_BLOCK(lfge_core, first_custom_type_id)

CAF_ADD_ATOM(lfge_core, lfge::core, heartbeat_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, timeout_atom)
CAF_ADD_ATOM(lfge_core, lfge::core, start_atom)

CAF_END_TYPE_ID_BLOCK(lfge_core)


namespace lfge::core{


    /**
     * @brief This class is used to create a heartbeat sender actor to any actor
     * Heartbeat sender sends an heartbeat_atom to a particular sender after each "frequency" time
     *  and waits for the reply of the heartbeat for a certain amount of "timeout" time
     *  There are N consecutive timeouts allowed, after N timeout the actor will self destruct
     *  For not having undefined behavior please set frequency > timeout atlease more than 25%
     */
    class HeartbeatSender : public caf::event_based_actor
    {
        std::string name;
        std::size_t frequency, timeout, n;
        std::size_t numberOfPastTimeouts;
        std::function<void(HeartbeatSender&)> onNTimeouts;
        std::function<void(HeartbeatSender&)> onHeartbeat, onTimeout, onRestart;
        caf::actor sendHeartbeatTo;

        std::atomic<bool> hbrecieved = false;
        std::size_t currentMessageIndex = 1;

        public:

        /**
         * @brief Construct a new Heartbeat Sender object
         * 
         * @param config -> Configuration class of CAF
         * @param sendHeartbeatTo -> Actor to send the heartbeat to. This actor should implement a behavior of type ( heartbeat_atom, std::size_t )
         * @param frequency -> Time duration in milliseconds at which heartbeats should be sent
         * @param timeout -> Time duration in milliseconds at which we can declare timeout
         * @param n -> After N consicutive failure we declare that actor to send is no longer available
         * @param actorName -> A unique identifier for actor (Optional)
         * @param onNTimeouts -> Method to be called after N timeouts to free the required resource (Optional)
         * @param onHeartbeat -> Function to call when we successfully get a heartbeat (Optional)
         * @param onTimeout -> Function to call when we successfullt get a timeout (Optional)
         * @param onRestart -> Function to call when we start a heartbeat (Optional)
         */
        HeartbeatSender( caf::actor_config& config,
                         caf::actor sendHeartbeatTo, 
                         const std::size_t& frequency, 
                         const std::size_t& timeout, 
                         const std::size_t& n,
                         std::string actorName = std::string(""),
                         std::function<void(HeartbeatSender&)> onNTimeouts = nullptr,
                         std::function<void(const HeartbeatSender&)> onHeartbeat = nullptr,
                         std::function<void(const HeartbeatSender&)> onTimeout = nullptr,
                         std::function<void(const HeartbeatSender&)> onRestart = nullptr );

        /**
         * @brief Get the name for the actor
         * 
         * @return const std::string& 
         */
        const std::string& getName() const;
protected:
        /**
         * @brief Overriding the make_behavior to implement require behavior
         * 
         * @return caf::behavior 
         */
        caf::behavior make_behavior() override;
    };
}