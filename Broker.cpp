#include "Broker.h"
#include "Except.h"
#include "MDP.h"
#include "utils.h"

#include <unistd.h>


constexpr
std::chrono::milliseconds Broker::timeout;

void Broker::exec(const std::string &address)
{
    for(;;)
    {
        zmqContextHandle_ = std::make_unique<ZMQContext>(ZMQIdentity::unique(), address);

        try
        {
            for(;;)
            {
                if(zmqContextHandle_->poller_.poll(timeout.count()))
                {
                    if(zmqContextHandle_->poller_.has_input(zmqContextHandle_->socket_))
                    {
                        auto message = recv(zmqContextHandle_->socket_, IOMode::NonBlockig);

                        if(message) onMessage(std::move(message));
                    }
                    /* in case of
                     * 1) high load (message traffic)
                     * 2) Worker and Broker heartbeats close timing
                     * timeout on poller wont happen so send heartbeat if needed */
                    sendHeartbeatIfNeeded();
                }
                else
                {
                    onTimeout();
                }
            }
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

void Broker::onMessage(MessageHandle handle)
{
    ASSERT(handle);

    try
    {
        /* at least 3 frames are required to deduce message type */
        ENSURE(3 <= handle->parts(), MessageFormatInvalid);
        /* Frame 0: identity */
        ENSURE(0 != handle->size(0), IdentityInvalid);
        /* Frame 1: empty */
        ENSURE(0 == handle->size(1), MessageFormatInvalid);
        /* Frame 2: six byte signature */
        ENSURE(6 == handle->size(2), MessageFormatInvalid);
    }
    catch(const EnsureException &except)
    {
        TRACE(this, ' ', except.invariant);
        dispatch(Tagged<Tag::Unsupported>{std::move(handle)});
        return;
    }

    if(MDP::Client::Signature::self == handle->get(2))
    {
        onClientMessage(std::move(handle));
    }
    else if(MDP::Worker::Signature::self == handle->get(2))
    {
        onWorkerMessage(std::move(handle));
    }
    else
    {
        ENSURE(false && "invalid signature", MessageFormatInvalid);
    }
}

void Broker::onClientMessage(MessageHandle handle)
{
    ASSERT(handle);
    TRACE(this, " ", handle);

    dispatch(Tagged<Tag::ClientRequest>(std::move(handle)));
}

void Broker::onWorkerMessage(MessageHandle handle)
{
    ASSERT(handle);
    ASSERT(4 <= handle->parts());

    // 0 - identity, 1 - empty frame, 2 - "MDPW01", 3 - signature
    const auto signature = handle->get(3);

    if(MDP::Worker::Signature::ready == signature)
    {
        dispatch(Tagged<Tag::WorkerReady>{std::move(handle)});
    }
    else if(MDP::Worker::Signature::reply == signature)
    {
        dispatch(Tagged<Tag::WorkerReply>{std::move(handle)});
    }
    else if(MDP::Worker::Signature::heartbeat == signature)
    {
        dispatch(Tagged<Tag::WorkerHeartbeat>{std::move(handle)});
    }
    else if(MDP::Worker::Signature::disconnect == signature)
    {
        dispatch(Tagged<Tag::WorkerDisconnect>{std::move(handle)});
    }
    else
    {
        dispatch(Tagged<Tag::Unsupported>{std::move(handle)});
    }
}

void Broker::dispatch(Tagged<Tag::ClientReply> tagged)
{
    TRACE(this, ' ', tagged.handle);
    send(zmqContextHandle_->socket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Broker::dispatch(Tagged<Tag::ClientRequest> tagged)
{
    TRACE(this, tagged.handle);
    ASSERT(3 <= tagged.handle->parts());

    using namespace MDP::Broker;

    const ZMQIdentity clientIdentity{tagged.handle->get(0)};

    if(4 > tagged.handle->parts())
    {
        dispatch(
            Tagged<Tag::ClientReply>(
                makeClientRep(clientIdentity, "", Signature::serviceUndefined)));
        return;
    }

    const auto serviceName = tagged.handle->get(3);

    if(!workerPool_.valid(serviceName))
    {
        dispatch(
            Tagged<Tag::ClientReply>(
                makeClientRep(clientIdentity, serviceName, Signature::serviceUnsupported)));
        return;
    }

    auto *worker = workerPool_.acquire(serviceName);

    if(nullptr == worker)
    {
        dispatch(
            Tagged<Tag::ClientReply>(
                makeClientRep(clientIdentity, serviceName, Signature::serviceBusy)));
        return;
    }

    auto request = makeWorkerReq(worker->identity_, clientIdentity);

    /* copy client request body (forward body to worker) */
    for(auto i = 4u; tagged.handle->parts() > i; ++i)
    {
        MDP::append(request, tagged.handle->get(i));
    }

    dispatch(Tagged<Tag::WorkerRequest>((std::move(request))), *worker);
}

void Broker::dispatch(Tagged<Tag::WorkerReady> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(5 == tagged.handle->parts());

    auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto serviceName = tagged.handle->get(4);

    TRACE(this, " ", workerIdentity.asString(), " ", serviceName,  " ready");

    workerPool_.append(serviceName, workerIdentity);
}

void Broker::dispatch(Tagged<Tag::WorkerRequest> tagged, WorkerPool::Worker &worker)
{
    TRACE(this, ' ', tagged.handle);

    worker.state_ = WorkerPool::Worker::State::Busy;
    worker.monitor_.selfHeartbeat();
    brokerTasks_.append(workerPool_.findWorker(worker.identity_));
    send(zmqContextHandle_->socket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Broker::dispatch(Tagged<Tag::WorkerReply> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(6 <= tagged.handle->parts());

    const auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto clientIdentity = ZMQIdentity{tagged.handle->get(4)};
    const auto workerIterator = brokerTasks_.workerIterator(workerIdentity);

    TRACE(
        this,
        " work ", workerIdentity.asString(),
        " serviceName ", workerIterator->serviceName_ ,
        " state ", workerIterator->state_,
        " reply");

    auto reply =
        MDP::Broker::makeClientRep(
            clientIdentity,
            workerIterator->serviceName_);

    /* copy worker reply body (forward body to worker) */
    for(auto i = 6u; tagged.handle->parts() > i; ++i)
    {
        MDP::append(reply, tagged.handle->get(i));
    }

    dispatch(Tagged<Tag::ClientReply>{std::move(reply)});

    workerIterator->monitor_.peerHeartbeat();
    workerIterator->state_ = WorkerPool::Worker::State::Idle;
    brokerTasks_.remove(workerIdentity);
}

void Broker::dispatch(Tagged<Tag::WorkerHeartbeat> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(0 < tagged.handle->parts());

    const auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto workerIterator = workerPool_.findWorker(workerIdentity);

    TRACE(
        this,
        " ", workerIdentity.asString(),
        " ", workerIterator->serviceName_ ,
        " ", workerIterator->state_,
        " ", workerIterator->monitor_,
        " heartbeat");

    workerIterator->monitor_.peerHeartbeat();
}

void Broker::dispatch(Tagged<Tag::WorkerDisconnect> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(0 < tagged.handle->parts());

    const auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto workerIterator = workerPool_.findWorker(workerIdentity);

    TRACE(
        this,
        " ", workerIdentity.asString(),
        " ", workerIterator->serviceName_ ,
        " ", workerIterator->state_,
        " disconnected");

    brokerTasks_.remove(workerIdentity);
    workerPool_.remove(workerIdentity);
}

void Broker::onTimeout()
{
    std::vector<ZMQIdentity> expired;

    workerPool_.forEachWorker(
        [this, &expired](WorkerPool::Worker &worker)
        {
            if(worker.monitor_.peerHeartbeatExpired())
            {
                TRACE(
                    this,
                    " ", worker.identity_.asString(),
                    " ", worker.serviceName_,
                    " ", worker.state_,
                    " heartbeat expired, disconnecting");
                send(
                    zmqContextHandle_->socket_,
                    MDP::Broker::makeDisconnect(worker.identity_),
                    IOMode::NonBlockig);

                expired.push_back(worker.identity_);
                return;
            }
        });

    for(const auto &identity : expired)
    {
        TRACE(this, " puring ", identity.asString());
        brokerTasks_.remove(identity);
        workerPool_.remove(identity);
    }

    sendHeartbeatIfNeeded();
}

void Broker::sendHeartbeatIfNeeded()
{
    workerPool_.forEachWorker(
        [this](WorkerPool::Worker &worker)
        {
            if(worker.monitor_.shouldHeartbeat())
            {
                send(
                    zmqContextHandle_->socket_,
                    MDP::Broker::makeHeartbeat(worker.identity_),
                    IOMode::NonBlockig);
                worker.monitor_.selfHeartbeat();
            }
        });
}

void Broker::dispatch(Tagged<Tag::Unsupported> tagged)
{
    ASSERT(tagged.handle);
    TRACE(this, " unsupported message discarded ", tagged.handle);
}


void help()
{
    std::cout
        << "broker -a broker_address"
        << std::endl;
}

int main(int argc, char *const argv[])
{
    std::string address;

    for(char c; -1 != (c = ::getopt(argc, argv, "ha:"));)
    {
        switch(c)
        {
            case 'h':
                help();
                return EXIT_SUCCESS;
                break;
            case 'a':
                address = optarg;
                break;
            case ':':
            case '?':
            default:
                return EXIT_FAILURE;
                break;
        }
    }

    if(address.empty())
    {
        help();
        return EXIT_FAILURE;
    }

    try
    {
        Broker broker;

        broker.exec(address);
    }
    catch(const EnsureException &except)
    {
        std::cerr << "ensure exception " << except.toString() << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception &except)
    {
        std::cerr << "std exception " << except.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        std::cerr << "unsupported exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
