#include "Heartbeat.hpp"

using namespace lfge::core;

HeartbeatSender::HeartbeatSender( caf::actor_config& config,
                        caf::actor sendHeartbeatTo, 
                        const std::size_t& frequency, 
                        const std::size_t& timeout, 
                        const std::size_t& n,
                        std::string actorName,
                        std::function<void(HeartbeatSender&)> onNTimeouts,
                        std::function<void(const HeartbeatSender&)> onHeartbeat,
                        std::function<void(const HeartbeatSender&)> onTimeout,
                        std::function<void(const HeartbeatSender&)> onRestart ) :
        event_based_actor(config), 
        sendHeartbeatTo(sendHeartbeatTo), 
        frequency(frequency), 
        timeout(timeout), 
        n(n),
        name(actorName), 
        numberOfPastTimeouts(0), 
        onNTimeouts(onNTimeouts),
        onHeartbeat(onHeartbeat),
        onTimeout(onTimeout),
        onRestart(onRestart)
{
    assert( frequency > timeout );
}


const std::string& HeartbeatSender::getName() const
{
    return name;
}

caf::behavior HeartbeatSender::make_behavior()
{
    if( timeout > frequency )
    {
        CAF_RAISE_ERROR(std::string("For a HeartbeatSender timeout cannot be greater than frequency (" + name + ")" ).c_str() );
        quit(  );
    }
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
            delayed_send(this, std::chrono::milliseconds(timeout), timeout_atom_v);
            //restart the counter after frequency milliseconds
            delayed_send(this, std::chrono::milliseconds(frequency), start_atom_v);
            send( sendHeartbeatTo, heartbeat_atom_v, ++currentMessageIndex );
        },
        // Called when heartbeat event is recived
        [&](heartbeat_atom, const std::size_t messageIndex )
        {
            if( messageIndex == currentMessageIndex )
            {
                hbrecieved = true;
                onNTimeouts = 0;
                if(onHeartbeat)
                {
                    onHeartbeat(*this);
                }
            }
        },
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
                    if(onNTimeouts)
                        onNTimeouts(*this);
                    quit();
                }
            }
        },
        [&](caf::error& e)
        {
            aout(this) << "Error on Heartbeat sender named " << name << ", Error Message " << to_string(e) << std::endl;
        }
    };
}