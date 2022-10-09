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

                }

                checkExpired();
                sendHeartbeatIfNeeded();
            }
        }
        catch(const std::exception &except)
        {
            TRACE(TraceLevel::Error, this, " ", except.what(), " restarting");
        }
        catch(...)
        {
            TRACE(TraceLevel::Error, this, " unsupported exception, restarting");
            return;
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
        TRACE(TraceLevel::Warning, this, ' ', except.invariant);
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
    TRACE(TraceLevel::Debug, this, " ", handle);

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
    TRACE(TraceLevel::Debug, this, ' ', tagged.handle);
    send(zmqContextHandle_->socket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Broker::dispatch(Tagged<Tag::ClientRequest> tagged)
{
    TRACE(TraceLevel::Debug, this, tagged.handle);
    ASSERT(3 <= tagged.handle->parts());

    using namespace MDP::Broker;

    const ZMQIdentity clientIdentity{tagged.handle->get(0)};

    if(4 > tagged.handle->parts())
    {
        dispatch(
            Tagged<Tag::ClientReply>(
                makeFailureClientRep(clientIdentity, "", Signature::serviceUndefined)));
        return;
    }

    const auto serviceName = tagged.handle->get(3);

    if(!workerPool_.valid(serviceName))
    {
        TRACE(TraceLevel::Warning, this, "service unsupported ", serviceName);
        dispatch(
            Tagged<Tag::ClientReply>(
                makeFailureClientRep(clientIdentity, serviceName, Signature::serviceUnsupported)));
        return;
    }

    auto *worker = workerPool_.acquire(serviceName);

    if(nullptr == worker)
    {
        dispatch(
            Tagged<Tag::ClientReply>(
                makeFailureClientRep(clientIdentity, serviceName, Signature::serviceBusy)));
        return;
    }

    auto request = makeWorkerReq(worker->identity_, clientIdentity);

    /* copy client request body (forward body to worker) */
    for(auto i = 4u; tagged.handle->parts() > i; ++i)
    {
        MDP::append(request, tagged.handle->get(i));
    }

    dispatch(Tagged<Tag::WorkerRequest>((std::move(request))), *worker, clientIdentity);
}

void Broker::dispatch(Tagged<Tag::WorkerReady> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(5 == tagged.handle->parts());

    auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto serviceName = tagged.handle->get(4);

    TRACE(TraceLevel::Info, this, " ", workerIdentity.asString(), " ", serviceName,  " ready");

    workerPool_.append(serviceName, workerIdentity);
}

void Broker::dispatch(
    Tagged<Tag::WorkerRequest> tagged,
    WorkerPool::Worker &worker,
    ZMQIdentity clientIdentity)
{
    TRACE(TraceLevel::Debug, this, ' ', tagged.handle);

    worker.state_ = WorkerPool::Worker::State::Busy;
    worker.monitor_.selfHeartbeat();
    brokerTasks_.append(workerPool_.findWorker(worker.identity_), clientIdentity);
    send(zmqContextHandle_->socket_, std::move(*tagged.handle), IOMode::Blocking);
}

void Broker::dispatch(Tagged<Tag::WorkerReply> tagged)
{
    ASSERT(tagged.handle);
    ASSERT(6 <= tagged.handle->parts());

    const auto workerIdentity = ZMQIdentity{tagged.handle->get(0)};
    const auto clientIdentity = ZMQIdentity{tagged.handle->get(4)};
    const auto &taskInfo = brokerTasks_.taskInfo(workerIdentity);
    const auto workerIterator = taskInfo.workerIterator_;

    TRACE(
        TraceLevel::Debug,
        this,
        " work ", workerIdentity.asString(),
        " serviceName ", workerIterator->serviceName_ ,
        " state ", workerIterator->state_,
        " reply");

    auto reply =
        MDP::Broker::makeSucessClientRep(
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
        TraceLevel::Debug,
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
        TraceLevel::Info,
        this,
        ", disconnecting: ", workerIdentity.asString(), ", ",
        workerIterator->serviceName_ , ", ",
        workerIterator->state_);

    if(brokerTasks_.valid(workerIdentity))
    {
        const auto &taskInfo = brokerTasks_.taskInfo(workerIdentity);

        dispatch(
            Tagged<Tag::ClientReply>(
                MDP::Broker::makeFailureClientRep(
                    taskInfo.clientIdentity_,
                    workerIterator->serviceName_,
                    MDP::Broker::Signature::serviceFailure)));
        brokerTasks_.remove(workerIdentity);
    }
    workerPool_.remove(workerIdentity);
}

void Broker::checkExpired()
{
    std::vector<ZMQIdentity> expired;

    workerPool_.forEachWorker(
        [this, &expired](WorkerPool::Worker &worker)
        {
            if(worker.monitor_.peerHeartbeatExpired())
            {
                TRACE(
                    TraceLevel::Warning,
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
        TRACE(TraceLevel::Warning, this, " purging ", identity.asString());

        const auto iterator = workerPool_.findWorker(identity);

        if(brokerTasks_.valid(identity))
        {
            const auto &taskInfo = brokerTasks_.taskInfo(identity);

            dispatch(
                Tagged<Tag::ClientReply>(
                    MDP::Broker::makeFailureClientRep(
                        taskInfo.clientIdentity_,
                        iterator->serviceName_,
                        MDP::Broker::Signature::serviceFailure)));
            brokerTasks_.remove(identity);
        }
        workerPool_.remove(identity);
    }

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
    TRACE(TraceLevel::Warning, this, " unsupported message discarded ", tagged.handle);
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

    for(int c; -1 != (c = ::getopt(argc, argv, "ha:"));)
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
    catch(const std::exception &except)
    {
        TRACE(TraceLevel::Error, " ", except.what());
        return EXIT_FAILURE;
    }
    catch(...)
    {
        TRACE(TraceLevel::Error, "unsupported exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
