#pragma once

#include <chrono>
#include <memory>

#include <zmqpp/zmqpp.hpp>

#include "MDP.h"
#include "ZMQIdentity.h"


struct ZMQWorkerContext
{
    zmqpp::context context_;
    zmqpp::socket socket_;
    zmqpp::socket masterSocket_;
    zmqpp::socket slaveSocket_;
    zmqpp::poller poller_;
    ZMQIdentity identity_;
    std::string address_;

    ZMQWorkerContext(ZMQIdentity identity, std::string address);
};
