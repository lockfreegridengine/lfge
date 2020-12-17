#include "Heartbeat.hpp"

using namespace lfge::core;

HeartbeatSender::HeartbeatSender( caf::actor_config& config,
                        caf::actor sendHeartbeatTo, 
                        const std::size_t& pulseDuration, 
                        const std::size_t& timeout, 
                        const std::size_t& n,
                        std::string actorName,
                        std::function<void(HeartbeatSender&)> onNTimeouts,
                        std::function<void(const HeartbeatSender&)> onHeartbeat,
                        std::function<void(const HeartbeatSender&)> onTimeout,
                        std::function<void(const HeartbeatSender&)> onRestart ) :
        event_based_actor(config), 
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


const std::string& HeartbeatSender::getName() const
{
    return name;
}

caf::behavior HeartbeatSender::make_behavior()
{
    if( timeout > pulseDuration )
    {
        CAF_RAISE_ERROR(std::string("For a HeartbeatSender timeout cannot be greater than pulseDuration (" + name + ")" ).c_str() );
        quit(  );
    }
    //Go to start state
    send( this, start_atom_v );

    return {
        // Called when we start doing heartbeat
        [&](start_atom)
        {
            if( onRestart )
            {
                onRestart(*this);
            }
            hbrecieved = false;
            (sendHeartbeatTo);
            //Send timeout event after timeout milliseconds
            delayed_send(this, clock_speed(timeout), timeout_atom_v);
            //restart the counter after pulseDuration milliseconds
            delayed_send(this, clock_speed(pulseDuration), start_atom_v);
            send( sendHeartbeatTo, heartbeat_atom_v, ++currentMessageIndex );
        },
        // Called when heartbeat reply is recived
        [&](heartbeat_reply_atom, const std::size_t messageIndex )
        {
            if( messageIndex == currentMessageIndex )
            {
                hbrecieved = true;
                numberOfPastTimeouts = 0;
                if(onHeartbeat)
                {
                    onHeartbeat(*this);
                }
            }
        },
        [&](heartbeat_atom, const std::size_t messageIndex)
        {
            send(this, heartbeat_reply_atom_v, messageIndex);
        }
        ,
        // called when timeout event is recieved
        [&](timeout_atom)
        {
            if( !hbrecieved )
            {
                if( onTimeout )
                {
                    onTimeout(*this);
                }

                ++numberOfPastTimeouts;
                if( numberOfPastTimeouts >= n )
                {
                    caf::aout(this) << "Timeout exceeded " << std::endl;
                    if(onNTimeouts)
                        onNTimeouts(*this);
                    quit();
                }
            }
        },
        //Returns current index
        [&](current_index_atom)
        {
            return currentMessageIndex;
        }
        ,
        [&](caf::error& e)
        {
            aout(this) << "Error on Heartbeat sender named " << name << ", Error Message " << to_string(e) << std::endl;
        }
    };
}



HeartbeatReciever::HeartbeatReciever(  caf::actor_config& config, 
                    std::string name,
                    std::function<void (HeartbeatReciever&, const std::size_t&)> onHeartbeat, 
                    std::size_t timeout, 
                    std::function<void (HeartbeatReciever&)> afterTimeout) : 
                    event_based_actor(config), name(name), onHeartbeat(onHeartbeat), timeout(timeout), afterTimeout(afterTimeout)
{

}

const std::string& HeartbeatReciever::getName() const
{
    return name;
}

caf::behavior HeartbeatReciever::make_behavior()
{
    if(timeout > 0)
    {
        delayed_send( this, clock_speed(timeout), timeout_atom_v );
    }
    this->set_default_handler(caf::drop);
    return { 
        [&](heartbeat_atom, const std::size_t messageIndex)
        {
            hasHeartbeat = true;
            onHeartbeat(*this, messageIndex);
            // TODO: Make the reciever skip message and go to the last one directly
            return caf::make_result( heartbeat_reply_atom_v, messageIndex );
        },
        [&](timeout_atom)
        {
            if( !hasHeartbeat )
            {
                afterTimeout(*this);
                quit();
            }
            hasHeartbeat = false;
            delayed_send( this, clock_speed(timeout), timeout_atom_v );
        } 
    };
}