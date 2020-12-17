#include "Heartbeat.hpp"

using namespace lfge::core;
using namespace caf;

HeartbeatSender::HeartbeatSender( actor_config& config,
                        actor sendHeartbeatTo, 
                        const std::size_t& pulseDuration, 
                        const std::size_t& timeout, 
                        const std::size_t& n,
                        std::string actorName,
                        std::function<void(HeartbeatSender&)> onNTimeouts,
                        std::function<void(const HeartbeatSender&)> onHeartbeat,
                        std::function<void(const HeartbeatSender&)> onTimeout,
                        std::function<void(const HeartbeatSender&)> onRestart ) :
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


const std::string& HeartbeatSender::getName() const
{
    return name;
}

HeartbeatSender::behavior_type HeartbeatSender::make_behavior()
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
        [this](start_atom)
        {
            if( onRestart )
            {
                onRestart(*this);
            }
            hbrecieved = false;
            //Send timeout event after timeout milliseconds
            delayed_send(this, clock_speed(timeout), timeout_atom_v);
            //restart the counter after pulseDuration milliseconds
            delayed_send(this, clock_speed(pulseDuration), start_atom_v);
            anon_send( sendHeartbeatTo, heartbeat_atom_v, ++currentMessageIndex );
        },
        // Called when heartbeat reply is recived
        [this](heartbeat_reply_atom, std::size_t messageIndex )
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
        // called when timeout event is recieved
        [this](timeout_atom)
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
        [this](current_index_atom) -> result<std::size_t>
        {
            return currentMessageIndex;
        },
        [this]( caf::error e )
        {
            aout(this) << "got error " << e;
        }
    };
}



HeartbeatReceiver::HeartbeatReceiver(  caf::actor_config& config, 
                    std::string name,
                    std::function<void (HeartbeatReceiver&, const std::size_t&)> onHeartbeat, 
                    std::size_t timeout, 
                    std::function<void (HeartbeatReceiver&)> afterTimeout) : 
                    typed_hb_receiver(config), name(name), onHeartbeat(onHeartbeat), timeout(timeout), afterTimeout(afterTimeout)
{

}

const std::string& HeartbeatReceiver::getName() const
{
    return name;
}

HeartbeatReceiver::behavior_type HeartbeatReceiver::make_behavior()
{
    if(timeout > 0)
    {
        delayed_send( this, clock_speed(timeout), timeout_atom_v );
    }
    this->set_default_handler(caf::drop);
    return { 
        [this](heartbeat_atom, std::size_t messageIndex) -> result<heartbeat_reply_atom, std::size_t>
        {
            hasHeartbeat = true;
            onHeartbeat(*this, messageIndex);
            // TODO: Make the reciever skip message and go to the last one directly
            return caf::make_result( heartbeat_reply_atom_v, messageIndex );
        },
        [this](timeout_atom)
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