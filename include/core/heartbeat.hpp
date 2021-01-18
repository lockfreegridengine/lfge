#pragma once

#include <chrono>
#include <cassert>
#include <exception>
#include "core/core_atoms.hpp"
namespace lfge::core{

    // Statically typed actor defination for heartbeat sender
    using typed_hb_sender = caf::typed_event_based_actor<
                                        caf::result<void>(lfge::core::start_atom),
                                        caf::result<std::size_t>(lfge::core::current_index_atom),
                                        caf::result<void>(const caf::error )
                                    >;

    // Statically typed actor defination for hb reciever
    using typed_hb_receiver = caf::typed_event_based_actor< 
                                caf::result<heartbeat_reply_atom, std::size_t>(heartbeat_atom, std::size_t)
                            >;

    /**
     * @brief This class is used to create a heartbeat sender actor to any actor
     * Heartbeat sender sends an heartbeat_atom to a particular sender after each "pulseDuration" time
     *  and waits for the reply of the heartbeat for a certain amount of "timeout" time
     *  There are N consecutive timeouts allowed, after N timeout the actor will self destruct
     *  For not having undefined behavior please set pulseDuration > timeout atlease more than 25%
     */
    class heartbeat_sender : public typed_hb_sender
    {
        std::string name;
        std::size_t pulseDuration, timeout, n;
        std::size_t numberOfPastTimeouts;
        std::function<void(heartbeat_sender&)> onNTimeouts;
        std::function<void(heartbeat_sender&)> onHeartbeat, onTimeout, onRestart;
        caf::actor sendHeartbeatTo;

        std::atomic<bool> hbrecieved = false;
        std::size_t currentMessageIndex = 1;

        public:

        using clock_speed = std::chrono::milliseconds;

        /**
         * @brief Construct a new Heartbeat Sender object
         * 
         * @param config -> Configuration class of CAF
         * @param sendHeartbeatTo -> Actor to send the heartbeat to. This actor should implement a behavior of type ( heartbeat_atom, std::size_t )
         * @param pulseDuration -> Time duration in milliseconds at which heartbeats should be sent
         * @param timeout -> Time duration in milliseconds at which we can declare timeout
         * @param n -> After N consicutive failure we declare that actor to send is no longer available
         * @param actorName -> A unique identifier for actor (Optional)
         * @param onNTimeouts -> Method to be called after N timeouts to free the required resource (Optional)
         * @param onHeartbeat -> Function to call when we successfully get a heartbeat (Optional)
         * @param onTimeout -> Function to call when we successfullt get a timeout (Optional)
         * @param onRestart -> Function to call when we start a heartbeat (Optional)
         */
        heartbeat_sender( caf::actor_config& config,
                         caf::actor sendHeartbeatTo, 
                         const std::size_t& pulseDuration, 
                         const std::size_t& timeout, 
                         const std::size_t& n,
                         std::string actorName = std::string(""),
                         std::function<void(heartbeat_sender&)> onNTimeouts = nullptr,
                         std::function<void(const heartbeat_sender&)> onHeartbeat = nullptr,
                         std::function<void(const heartbeat_sender&)> onTimeout = nullptr,
                         std::function<void(const heartbeat_sender&)> onRestart = nullptr );

        /**
         * @brief Get the name for the actor
         * 
         * @return const std::string& 
         */
        const std::string& getName() const;

        /**
         * @brief Overriding the make_behavior to implement require behavior
         * 
         * @return caf::behavior 
         */
        behavior_type make_behavior() override;
    };

    /**
     * @brief This class replies to a given heartbeat event sent by heartbeeat sender
     * Optionally we can give a timeout if no heartbeat event is recieved for timeout time the actor will quit automatically
     * It contains a name field to give unique name to the reciever.
     * onHeartbeat functor which is called on each heartbeat
     * afterTimeout functor which is called after a timeout
     * 
     */
    class heartbeat_receiver : public typed_hb_receiver
    {
        std::string name;
        std::function<void (heartbeat_receiver&)> onHeartbeat;
        std::function<void (heartbeat_receiver&)> afterTimeout;
        std::size_t timeout;
        std::atomic<bool> hasHeartbeat = false;

        public:

        using clock_speed = std::chrono::milliseconds;

        /**
         * @brief Construct a new Heartbeat Reciever object
         * 
         * @param config -> Internal config used by CAF
         * @param name -> name of the reciever
         * @param onHeartbeat -> Functor called on heartbeat
         * @param timeout -> timeout in milliseconds (0 for no timeout)
         * @param afterTimeout -> Functor called after timeout
         */
        heartbeat_receiver(  caf::actor_config& config, 
                            std::string name = "",
                            std::function<void (heartbeat_receiver&)> onHeartbeat = nullptr, 
                            std::size_t timeout = 0, 
                            std::function<void (heartbeat_receiver&)> afterTimeout = nullptr);
        

        /**
         * @brief Get the name for the actor
         * 
         * @return const std::string& 
         */
        const std::string& getName() const;

        /**
         * @brief Overriding the make_behavior to implement require behavior
         * 
         * @return caf::behavior 
         */
        behavior_type make_behavior() override;
    };
}