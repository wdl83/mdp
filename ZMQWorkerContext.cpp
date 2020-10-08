#include "Ensure.h"
#include "Except.h"
#include "ZMQWorkerContext.h"


ZMQWorkerContext::ZMQWorkerContext(ZMQIdentity identity, std::string address):
    socket_{context_, zmqpp::socket_type::dealer},
    masterSocket_{context_, zmqpp::socket_type::dealer},
    slaveSocket_{context_, zmqpp::socket_type::dealer},
    identity_{std::move(identity)},
    address_{std::move(address)}
{
    ENSURE(context_, RuntimeError);
    ENSURE(!address_.empty(), RuntimeError);

    socket_.set(zmqpp::socket_option::identity, identity_.data(), identity_.size());
    socket_.set(zmqpp::socket_option::linger, 0);

    const char slaveIdentity[] = "slave";
    const char masterIdentity[] = "master";
    const char masterAddress[] = "inproc://master";

    masterSocket_.set(zmqpp::socket_option::identity, masterIdentity, sizeof(masterIdentity));
    masterSocket_.set(zmqpp::socket_option::linger, 0);

    slaveSocket_.set(zmqpp::socket_option::identity, slaveIdentity, sizeof(slaveIdentity));
    slaveSocket_.set(zmqpp::socket_option::linger, 0);

    masterSocket_.bind(masterAddress);
    slaveSocket_.connect(masterAddress);

    socket_.connect(address_);
    poller_.add(socket_, zmqpp::poller::poll_in | zmqpp::poller::poll_error);
    poller_.add(masterSocket_, zmqpp::poller::poll_in | zmqpp::poller::poll_error);
}
