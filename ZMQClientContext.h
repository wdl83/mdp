#pragma once

#include <chrono>
#include <memory>

#include <zmqpp/zmqpp.hpp>

#include "MDP.h"
#include "ZMQIdentity.h"


struct ZMQClientContext
{
    using Message = MDP::Message;
    using MessageHandle = MDP::MessageHandle;
private:
    zmqpp::context context_;
    zmqpp::socket socket_;
    ZMQIdentity identity_;
    std::string address_;
public:
    ZMQClientContext(ZMQIdentity identity, std::string address);
    Message recv();
    void send(Message);
    const ZMQIdentity &identity() const {return identity_;}
};
