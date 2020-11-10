#pragma once

#include "MDP.h"
#include "ZMQClientContext.h"


class Client
{
    using ZMQContext = ZMQClientContext;
public:
    using Message = MDP::Message;
    using PayloadSeq = std::vector<std::string>;

    PayloadSeq exec(
        const std::string &address,
        const std::string &serviceName,
        const PayloadSeq &payload);
private:
    void onRequest(Message, ZMQContext &);
    Message onMessage(Message, const ZMQContext &, const std::string &);
    PayloadSeq onReply(Message);

};
