#include "core/heartbeat.hpp"
#include "core/logger.hpp"

using namespace lfge::core;
using namespace caf;

heartbeat_sender::heartbeat_sender( actor_config& config,
                        actor sendHeartbeatTo, 
                        const std::size_t& pulseDuration, 
                        const std::size_t& timeout, 
                        const std::size_t& n,
                        std::string actorName,
                        std::function<void(heartbeat_sender&)> onNTimeouts,
                        std::function<void(const heartbeat_sender&)> onHeartbeat,
                        std::function<void(const heartbeat_sender&)> onTimeout,
                        std::function<void(const heartbeat_sender&)> onRestart ) :
        typed_hb_sender(config), 
        sendHeartbeatTo(sendHeartbeatTo), 
        pulseDuration(pulseDuration), 
        timeout(timeout), 
        n(n),
        name(std::move(actorName)), 
        numberOfPastTimeouts(0), 
        onNTimeouts(onNTimeouts),
        onHeartbeat(onHeartbeat),
        onTimeout(onTimeout),
        onRestart(onRestart)
{
    assert( pulseDuration > timeout );
}


const std::string& heartbeat_sender::getName() const
{
    return name;
}

heartbeat_sender::behavior_type heartbeat_sender::make_behavior()
{
    if( timeout > pulseDuration )
    {
        std::string error("For a heartbeat_sender timeout cannot be greater than pulseDuration (" + name + ")" );
        logger::log( loglevel::error, getName() + " heartbeat sender for error " + error );
        CAF_RAISE_ERROR(error.c_str() );
        quit(  );
    }
    //Go to start state
    delayed_send( this, clock_speed(pulseDuration), start_atom_v );
    set_default_handler(caf::print_and_drop);

    return {
        // Called when we start doing heartbeat
        [this](start_atom)
        {
            logger::log( loglevel::comm, getName() + " sending hb signal" );

            if( onRestart )
            {
                onRestart(*this);
            }
            hbrecieved = false;
            //restart the counter after pulseDuration milliseconds
            delayed_send(this, clock_speed(pulseDuration), start_atom_v);

            //Request heartbeat to the sender
            request( sendHeartbeatTo, clock_speed(timeout), heartbeat_atom_v, ++currentMessageIndex )
                .then( 
                    [this]( heartbeat_reply_atom, std::size_t messageIndex )
                    {
                        if( messageIndex == currentMessageIndex )
                        {
                            logger::log( loglevel::comm, getName() + " got a hb reply " );
                            hbrecieved = true;
                            numberOfPastTimeouts = 0;
                            if(onHeartbeat)
                            {
                                onHeartbeat(*this);
                            }
                        }
                    },
                    [this]( const error& err )
                    {
                        logger::log( loglevel::warning, getName() + " heartbeat sender got error " + to_string(err) );
                        if( onTimeout )
                        {
                            onTimeout(*this);
                        }

                        ++numberOfPastTimeouts;
                        if( numberOfPastTimeouts >= n )
                        {
                            if(onNTimeouts)
                                onNTimeouts(*this);
                            quit();
                        }
                    }
            );
        },
        //Returns current index
        [this](current_index_atom) -> result<std::size_t>
        {
            return currentMessageIndex;
        },
        [this]( const caf::error e )
        {
            logger::log( loglevel::error, getName() + " received an error message " + to_string(e) );
        }
    };
}



heartbeat_receiver::heartbeat_receiver(  caf::actor_config& config, 
                    std::string name,
                    std::function<void (heartbeat_receiver&)> onHeartbeat, 
                    std::size_t timeout, 
                    std::function<void (heartbeat_receiver&)> afterTimeout) : 
                    typed_hb_receiver(config), name(name), onHeartbeat(onHeartbeat), timeout(timeout), afterTimeout(afterTimeout)
{

}

const std::string& heartbeat_receiver::getName() const
{
    return name;
}

heartbeat_receiver::behavior_type heartbeat_receiver::make_behavior()
{
    this->set_default_handler(caf::drop);
    return { 
        [this](heartbeat_atom, std::size_t messageIndex) -> result<heartbeat_reply_atom, std::size_t>
        {
            logger::log( loglevel::comm, getName() + " received an hb message ");
            
            if(onHeartbeat)
            {
                onHeartbeat(*this);
            }

            return caf::make_result( heartbeat_reply_atom_v, messageIndex );
        },
        after( timeout == 0 ? infinite : std::chrono::milliseconds(timeout) ) >> 
        [this]()
        {
            logger::log( loglevel::warning, getName() + " to timeout while waiting for messages ");
            afterTimeout(*this);
            quit();
        } 
    };
}