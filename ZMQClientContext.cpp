#include "Ensure.h"
#include "Except.h"
#include "ZMQClientContext.h"


ZMQClientContext::ZMQClientContext(ZMQIdentity identity, std::string address):
    socket_{context_, zmqpp::socket_type::dealer},
    identity_{std::move(identity)},
    address_{std::move(address)}
{
    ENSURE(context_, RuntimeError);
    ENSURE(!address_.empty(), RuntimeError);

    socket_.set(zmqpp::socket_option::identity, identity_.data(), identity_.size());
    socket_.set(zmqpp::socket_option::linger, 0);

    socket_.connect(address_);
}

auto ZMQClientContext::recv() -> Message
{
    Message msg;
    const auto status = socket_.receive(msg, false /* dont_block */);

    ENSURE(status, RecvFailed);
    return msg;
}

void ZMQClientContext::send(Message msg)
{
    const auto status = socket_.send(msg, false /* dont_block */);

    ENSURE(status, SendFailed);
}
