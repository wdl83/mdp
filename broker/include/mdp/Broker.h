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

#include "mdp/BrokerTasks.h"
#include "mdp/MDP.h"
#include "mdp/WorkerPool.h"
#include "mdp/ZMQBrokerContext.h"
#include "mdp/ZMQIdentity.h"

struct Broker
{

    using MessageHandle    = MDP::MessageHandle;
    using ZMQContext       = ZMQBrokerContext;
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

        Tagged() { }
        Tagged(MessageHandle h)
            : handle{std::move(h)}
        { }
        Tagged(zmqpp::message m)
            : handle{MDP::makeMessageHandle(std::move(m))}
        { }
    };

    Broker(std::chrono::milliseconds timeout = std::chrono::seconds{3});
    void exec(const std::string &address);
private:
    std::chrono::milliseconds timeout_;
    ZMQContextHandle zmqContextHandle_{};
    WorkerPool workerPool_{};
    BrokerTasks brokerTasks_{};

    void onMessage(MessageHandle);
    void onClientMessage(MessageHandle);
    void onWorkerMessage(MessageHandle);
    /* Client */
    void dispatch(Tagged<Tag::ClientRequest>);
    void dispatch(Tagged<Tag::ClientReply>);
    /* Worker */
    void dispatch(Tagged<Tag::WorkerReady>);
    void dispatch(
        Tagged<Tag::WorkerRequest>,
        WorkerPool::Worker &,
        ZMQIdentity clientIdentity);
    void dispatch(Tagged<Tag::WorkerReply>);
    void dispatch(Tagged<Tag::WorkerHeartbeat>);
    void dispatch(Tagged<Tag::WorkerDisconnect>);
    /* Misc */
    void checkExpired();
    void purge(ZMQIdentity);
    void sendHeartbeatIfNeeded();
    void dispatch(Tagged<Tag::Unsupported>);
};
