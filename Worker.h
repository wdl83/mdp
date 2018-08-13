#pragma once

#include "MDP.h"
#include "MutualHeartbeatMonitor.h"
#include "WorkerTask.h"
#include "ZMQWorkerContext.h"


class Worker
{
    using Message = MDP::Message;
    using MessageHandle = MDP::MessageHandle;
    using ZMQContext = ZMQWorkerContext;

    static
    constexpr std::chrono::milliseconds timeout = MutualHeartbeatMonitor::period;
public:
    void exec(
        const std::string &address,
        const std::string &serviceName,
        WorkerTask::Transform);
private:
    MutualHeartbeatMonitor monitor_;

    enum class Tag
    {
        ClientRequest,
        ClientResponse,
        BrokerHeartbeat,
        BrokerDisconnect,
        Unsupported
    };

    template <Tag value>
    struct Tagged
    {
        static constexpr Tag tag = value;

        MessageHandle handle;

        Tagged() {}
        Tagged(MessageHandle h): handle{std::move(h)} {}
        Tagged(Message m): handle{MDP::makeMessageHandle(std::move(m))} {}
    };

    void exec(ZMQContext &, const std::string &);
    void registerService(ZMQContext &, const std::string &);
    void provideService(ZMQContext &, const std::string &);
    void onMessage(ZMQContext &, MessageHandle);
    void onTaskMessage(ZMQContext &, MessageHandle);
    void onTimeout(ZMQContext &);
    void sendHeartbeatIfNeeded(ZMQContext &);
    /* Client */
    void dispatch(ZMQContext &, Tagged<Tag::ClientRequest>);
    void dispatch(ZMQContext &, Tagged<Tag::ClientResponse>);
    void dispatch(ZMQContext &, Tagged<Tag::BrokerHeartbeat>);
    void dispatch(ZMQContext &, Tagged<Tag::BrokerDisconnect>);
    void dispatch(ZMQContext &, Tagged<Tag::Unsupported>);
};
