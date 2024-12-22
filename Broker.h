#pragma once

// https://rfc.zeromq.org/spec:7/MDP/

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "BrokerTasks.h"
#include "MDP.h"
#include "WorkerPool.h"
#include "ZMQBrokerContext.h"
#include "ZMQIdentity.h"


struct Broker
{
    static
    constexpr std::chrono::milliseconds timeout = MutualHeartbeatMonitor::period;

    using MessageHandle = MDP::MessageHandle;
    using ZMQContext = ZMQBrokerContext;
    using ZMQContextHandle = std::unique_ptr<ZMQContext>;

    enum class Tag
    {
        ClientRequest,
        ClientReply,
        WorkerReady,
        WorkerRequest,
        WorkerReply,
        WorkerHeartbeat,
        WorkerDisconnect,
        Unsupported
    };

    template <Tag value>
    struct Tagged
    {
        static constexpr Tag tag = value;

        MessageHandle handle;

        Tagged() {}
        Tagged(MessageHandle h): handle{std::move(h)} {}
        Tagged(zmqpp::message m): handle{MDP::makeMessageHandle(std::move(m))} {}
    };

    void exec(const std::string &address);
private:
    ZMQContextHandle zmqContextHandle_;
    WorkerPool workerPool_;
    BrokerTasks brokerTasks_;

    void onMessage(MessageHandle);
    void onClientMessage(MessageHandle);
    void onWorkerMessage(MessageHandle);
    /* Client */
    void dispatch(Tagged<Tag::ClientRequest>);
    void dispatch(Tagged<Tag::ClientReply>);
    /* Worker */
    void dispatch(Tagged<Tag::WorkerReady>);
    void dispatch(Tagged<Tag::WorkerRequest>, WorkerPool::Worker &, ZMQIdentity clientIdentity);
    void dispatch(Tagged<Tag::WorkerReply>);
    void dispatch(Tagged<Tag::WorkerHeartbeat>);
    void dispatch(Tagged<Tag::WorkerDisconnect>);
    /* Misc */
    void checkExpired();
    void purge(ZMQIdentity);
    void sendHeartbeatIfNeeded();
    void dispatch(Tagged<Tag::Unsupported>);
};
