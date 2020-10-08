#include "ZMQBrokerContext.h"
#include "Except.h"


ZMQBrokerContext::ZMQBrokerContext(ZMQIdentity identity, std::string address):
    socket_{context_, zmqpp::socket_type::router},
    identity_{std::move(identity)},
    address_{std::move(address)}
{
    ENSURE(context_, RuntimeError);
    ENSURE(!address_.empty(), RuntimeError);

    socket_.set(zmqpp::socket_option::identity, identity_.data(), identity_.size());
    socket_.set(zmqpp::socket_option::linger, 0);

    socket_.bind(address_);
    poller_.add(socket_, zmqpp::poller::poll_in | zmqpp::poller::poll_error);
}
