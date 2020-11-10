#include "Client.h"
#include "Except.h"

auto Client::exec(
    const std::string &address,
    const std::string &serviceName,
    const PayloadSeq &payloadSeq) -> PayloadSeq
{
    auto zmqContext = ZMQContext{ZMQIdentity::unique(), address};

    try
    {
        onRequest(MDP::Client::makeReq(serviceName, payloadSeq), zmqContext);
        return onReply(onMessage(zmqContext.recv(), zmqContext, serviceName));
    }
    catch(const std::exception &except)
    {
        TRACE(TraceLevel::Error, this, " ", except.what(), " aborting");
        return {};
    }
    catch(...)
    {
        TRACE(TraceLevel::Error, this, " Unsupported exception, aborting");
        return {};
    }

    return {};
}

void Client::onRequest(Message message, ZMQContext &zmqContext)
{
    TRACE(TraceLevel::Debug, this, " ", message);
    zmqContext.send(std::move(message));
}

auto Client::onMessage(
    Message msg,
    const ZMQContext &,
    const std::string &serviceName) -> Message
{
    ENSURE(4 <= msg.parts(), MessageFormatInvalid);
    /* Frame 0: empty */
    ENSURE(0 == msg.size(0), MessageFormatInvalid);
    /* Frame 1: six byte signature (client) */
    ENSURE(6 == msg.size(1), MessageFormatInvalid);
    ENSURE(MDP::Client::Signature::self == msg.get(1), MessageFormatInvalid);
    /* Frame 2: service name */
    ENSURE(serviceName == msg.get(2), MessageFormatInvalid);
    /* Frame 3+: payload */
    ENSURE(0 < msg.size(3), MessageFormatInvalid);
    return msg;
}

auto Client::onReply(Message message) ->PayloadSeq
{
    TRACE(TraceLevel::Debug, this, " ", message);

    PayloadSeq seq;

    for(size_t i = 3; i < message.parts(); ++i) seq.push_back(message.get(i));
    return seq;
}
