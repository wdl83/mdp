#pragma once

#include <chrono>
#include <memory>

#include <zmqpp/zmqpp.hpp>

#include "ZMQIdentity.h"


struct ZMQBrokerContext
{
    zmqpp::context context_;
    zmqpp::socket socket_;
    zmqpp::poller poller_;
    ZMQIdentity identity_;
    std::string address_;

    ZMQBrokerContext(ZMQIdentity identity, std::string address);
};
