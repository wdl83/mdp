#include "Worker.h"
#include "utils.h"
#include "Except.h"

#include <future>


constexpr
std::chrono::milliseconds Worker::timeout;

void Worker::exec(
    const std::string &address,
    const std::string &serviceName,
    WorkerTask::Transform transform)
{
    for(;;)
    {
        try
        {
            monitor_.reset();

            TRACE(this, " service ", serviceName, " broker ", address);

            auto zmqContext = ZMQContext{ZMQIdentity::unique(), address};

            auto r =
                std::async(
                    std::launch::async,
                    [&transform, &zmqContext]()
                    {
                        WorkerTask{transform}(zmqContext.slaveSocket_);
                    });

            WorkerTask::Guard rGuard{zmqContext.masterSocket_};

            exec(zmqContext, serviceName);
        }
        catch(const EnsureException &except)
        {
            TRACE(this, " EnsureException: ", except.toString(), " restarting");
        }
        catch(const std::exception &except)
        {
            TRACE(this, " std::exception: ", except.what(), " restarting");
        }
        catch(...)
        {
            TRACE(this, " Unsupported exception, restarting");
        }
    }
}

void Worker::exec(ZMQContext &zmqContext, const std::string &serviceName)
{
    try
    {
        registerService(zmqContext, serviceName);
        provideService(zmqContext, serviceName);
    }
    catch(const EnsureException &except)
    {
        TRACE(this, " EnsureException: ", except.toString(), " restarting");
    }
    catch(const std::exception &except)
    {
        TRACE(this, " std::exception: ", except.what(), " restarting");
    }
    catch(...)
    {
        TRACE(this, " Unsupported exception, restarting");
    }
}

void Worker::registerService(ZMQContext &zmqContext, const std::string &serviceName)
{
    auto ready = MDP::Worker::makeReady(serviceName);

    TRACE(this, " ", serviceName, " ", ready);

    send(zmqContext.socket_, std::move(ready), IOMode::Blocking);
    monitor_.selfHeartbeat();
}

void Worker::provideService(ZMQContext &zmqContext, const std::string &serviceName)
{
    TRACE(this, " ", serviceName);

    for(uint64_t cntr = 0;; ++cntr)
    {
       // TRACE(this, " [", cntr, "] waiting");

        if(zmqContext.poller_.poll(timeout.count()))
        {
            if(zmqContext.poller_.has_input(zmqContext.socket_))
            {
                auto handle = recv(zmqContext.socket_, IOMode::NonBlockig);

                if(handle) onMessage(zmqContext, std::move(handle));
            }
            else if(zmqContext.poller_.has_input(zmqContext.masterSocket_))
            {
                auto handle = recv(zmqContext.masterSocket_, IOMode::NonBlockig);

                if(handle) onTaskMessage(zmqContext, std::move(handle));
            }
            else
            {
                ENSURE(false && " not supported", FlowError);
            }

            /* in case of
             * 1) high load (message traffic)
             * 2) Worker and Broker heartbeats close timing
             * timeout on poller wont happen so send heartbeat if needed */
            sendHeartbeatIfNeeded(zmqContext);
        }
        else
        {
            onTimeout(zmqContext);
        }
    }
}

void Worker::onMessage(ZMQContext &zmqContext, MessageHandle handle)
{
    ASSERT(handle);

    try
    {
        ENSURE(3 <= handle->parts(), MessageFormatInvalid);
        /* Frame 0: empty */
        ENSURE(0 == handle->size(0), MessageFormatInvalid);
        /* Frame 1: six byte signature (worker) */
        ENSURE(6 == handle->size(1), MessageFormatInvalid);
        ENSURE(MDP::Worker::Signature::self == handle->get(1), MessageFormatInvalid);
    }
    catch(const EnsureException &except)
    {
        dispatch(zmqContext, Tagged<Tag::Unsupported>{std::move(handle)});
        return;
    }

    if(MDP::Worker::Signature::request == handle->get(2))
    {
        dispatch(zmqContext, Tagged<Tag::ClientRequest>{std::move(handle)});
    }
    else if(MDP::Worker::Signature::heartbeat == handle->get(2))
    {
        dispatch(zmqContext, Tagged<Tag::BrokerHeartbeat>{std::move(handle)});
    }
    else if(MDP::Worker::Signature::disconnect == handle->get(2))
    {
        dispatch(zmqContext, Tagged<Tag::BrokerDisconnect>{std::move(handle)});
    }
    else
    {
        dispatch(zmqContext, Tagged<Tag::Unsupported>{std::move(handle)});
    }
}

void Worker::onTaskMessage(ZMQContext &zmqContext, MessageHandle handle)
{
    ASSERT(handle);
    TRACE(this, " ", handle);
    dispatch(zmqContext, Tagged<Tag::ClientResponse>{std::move(handle)});
}

void Worker::onTimeout(ZMQContext &zmqContext)
{
    /* check broker heartbeating, if expired abort and restart */
    ENSURE(!monitor_.peerHeartbeatExpired(), BrokerHeartbeatExpired);
    sendHeartbeatIfNeeded(zmqContext);
}

void Worker::sendHeartbeatIfNeeded(ZMQContext &zmqContext)
{
    if(monitor_.shouldHeartbeat())
    {
        send(zmqContext.socket_, MDP::Worker::makeHeartbeat(), IOMode::NonBlockig);
        monitor_.selfHeartbeat();
    }
}

void Worker::dispatch(ZMQContext &zmqContext, Tagged<Tag::ClientRequest> tagged)
{
    ASSERT(tagged.handle);
    TRACE(this, " ", tagged.handle);
    /* every valid message received is treated as peers heartbeat */
    monitor_.peerHeartbeat();
    send(zmqContext.masterSocket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Worker::dispatch(ZMQContext &zmqContext, Tagged<Tag::ClientResponse> tagged)
{
    ASSERT(tagged.handle);
    TRACE(this, " ", tagged.handle);
    send(zmqContext.socket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Worker::dispatch(ZMQContext &, Tagged<Tag::BrokerHeartbeat> tagged)
{
    ASSERT(tagged.handle);
    TRACE(this, " ", monitor_);
    monitor_.peerHeartbeat();
}

void Worker::dispatch(ZMQContext &, Tagged<Tag::BrokerDisconnect> tagged)
{
    ASSERT(tagged.handle);
    ENSURE(false && " disconnected by broker", BrokerDisconnected);
}

void Worker::dispatch(ZMQContext &, Tagged<Tag::Unsupported> tagged)
{
    ASSERT(tagged.handle);
    TRACE(this, " unsupported message discarded ", tagged.handle);
}
