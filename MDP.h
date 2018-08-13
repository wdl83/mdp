#pragma once

// https://rfc.zeromq.org/spec:7/MDP/

#include <zmqpp/zmqpp.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Ensure.h"
#include "ZMQIdentity.h"


namespace MDP {

struct EmptyFrame {};
using Message = zmqpp::message;
using MessageHandle = std::unique_ptr<Message>;

template <typename ...T_n>
MessageHandle makeMessageHandle(T_n &&...tail)
{
    return std::make_unique<Message>(std::forward<T_n>(tail)...);
}

inline
void append(Message &) {}
template <typename T, typename ...T_n>
void append(Message &msg, const T &value, const T_n &...tail);
template <typename ...T_n>
void append(Message &msg, const EmptyFrame &, const T_n &...tail);
template <typename ...T_n>
void append(Message &msg, const ZMQIdentity &identity, const T_n &...tail);
template <typename ...T_n>
void append(Message &msg, const std::vector<std::string> &seq, const T_n &...tail);

template <typename T, typename ...T_n>
void append(Message &msg, const T &value, const T_n &...tail)
{
    msg.add(value);
    append(msg, tail...);
}

template <typename ...T_n>
void append(Message &msg, const EmptyFrame &, const T_n &...tail)
{
    msg.raw_new_msg();
    append(msg, tail...);
}

template <typename ...T_n>
void append(Message &msg, const ZMQIdentity &identity, const T_n &...tail)
{
    msg.add_raw(identity.data(), identity.size());
    append(msg, tail...);
}

template <typename ...T_n>
void append(Message &msg, const std::vector<std::string> &seq, const T_n &...tail)
{
    for(const auto &str : seq) msg.add(str);
    append(msg, tail...);
}

template <typename ...T_n>
Message makeMessage(const T_n &...tail)
{
    Message msg;

    append(msg, tail...);
    return msg;
}

namespace Client {

namespace Signature {
static constexpr auto self = "MDPC01";
} /* Signature */

/* Client REQUEST:
 *  Frame 0: Empty (zero bytes, invisible to REQ application)
 *  Frame 1: "MDPC01" (six bytes, representing MDP/Client v0.1)
 *  Frame 2: Service name (printable string)
 *  Frames 3+: Request body (opaque binary) */
template <typename ...T_n>
Message makeReq(const std::string &service, const T_n & ...body)
{
    return makeMessage(EmptyFrame{}, Signature::self, service, body...);
}

} /* Client */

namespace Worker {

namespace Signature {
static constexpr auto self = "MDPW01";
static constexpr auto ready = "\x1";
static constexpr auto request = "\x2";
static constexpr auto reply = "\x3";
static constexpr auto heartbeat = "\x4";
static constexpr auto disconnect = "\x5";
} /* Signature */

/* Worker READY
 *  Frame 0: Empty frame
 *  Frame 1: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 2: 0x01 (one byte, representing READY)
 *  Frame 3: Service name (printable string) */
inline
Message makeReady(const std::string &service)
{
    return makeMessage(EmptyFrame{}, Signature::self, Signature::ready, service);
}

/* Worker  REPLY
 *  Frame 0: Empty frame
 *  Frame 1: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 2: 0x03 (one byte, representing REPLY)
 *  Frame 3: Client address (envelope stack)
 *  Frame 4: Empty (zero bytes, envelope delimiter)
 *  Frames 5+: Reply body (opaque binary) */
template <typename ...T_n>
Message makeRep(const ZMQIdentity &identity, const T_n & ...body)
{
    return makeMessage(EmptyFrame{}, Signature::self, Signature::reply, identity, EmptyFrame{}, body...);
}

/* Worker HEARTBEAT
 *  Frame 0: Empty frame
 *  Frame 1: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 2: 0x04 (one byte, representing HEARTBEAT) */
inline
Message makeHeartbeat()
{
    return makeMessage(EmptyFrame{}, Signature::self, Signature::heartbeat);
}

/* Worker DISCONNECT
 *  Frame 0: Empty frame
 *  Frame 1: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 2: 0x05 (one byte, representing DISCONNECT) */
inline
Message makeDisconnect()
{
    return makeMessage(EmptyFrame{}, Signature::self, Signature::disconnect);
}

} /* Worker */

namespace Broker {

namespace Signature {
static constexpr auto serviceUndefined = "error: service undefined";
static constexpr auto serviceUnsupported = "service unsupported";
static constexpr auto serviceBusy = "service busy";
static constexpr auto serviceRegistered = "service registered";
}

/* Client REPLY:
 *  Frame 0: Identity
 *  Frame 1: Empty (zero bytes, invisible to REQ application)
 *  Frame 2: "MDPC01" (six bytes, representing MDP/Client v0.1)
 *  Frame 3: Service name (printable string)
 *  Frames 4+: Reply body (opaque binary) */
template <typename ...T_n>
Message makeClientRep(const ZMQIdentity &identity, const std::string &service, const T_n & ...body)
{
    return makeMessage(identity, EmptyFrame{}, Client::Signature::self, service, body...);
}

/* Worker REQUEST
 *  Frame 0: Identity
 *  Frame 1: Empty frame
 *  Frame 2: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 3: 0x02 (one byte, representing REQUEST)
 *  Frame 4: Client address (envelope stack)
 *  Frame 5: Empty (zero bytes, envelope delimiter)
 *  Frames 6+: Request body (opaque binary) */
template <typename ...T_n>
Message makeWorkerReq(const ZMQIdentity &workerIdentity, const ZMQIdentity &clientIdentity, const T_n & ...body)
{
    return makeMessage(workerIdentity, EmptyFrame{}, Worker::Signature::self, Worker::Signature::request, clientIdentity, EmptyFrame{}, body...);
}

/* Worker HEARTBEAT
 *  Frame 0: Identity
 *  Frame 1: Empty frame
 *  Frame 2: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 3: 0x04 (one byte, representing HEARTBEAT) */
inline
Message makeHeartbeat(const ZMQIdentity &identity)
{
    return makeMessage(identity, EmptyFrame{}, Worker::Signature::self, Worker::Signature::heartbeat);
}

/* Worker DISCONNECT
 *  Frame 0: Identity
 *  Frame 1: Empty frame
 *  Frame 2: "MDPW01" (six bytes, representing MDP/Worker v0.1)
 *  Frame 3: 0x05 (one byte, representing DISCONNECT) */
inline
Message makeDisconnect(const ZMQIdentity &identity)
{
    return makeMessage(identity, EmptyFrame{}, Worker::Signature::self, Worker::Signature::disconnect);
}

} /* Broker */
} /* MDP */

/* ADL for zmqpp::message */
namespace zmqpp {

inline
void traceSerializeImpl(std::ostream &os, const MDP::Message &message)
{
    os << '{';

    for(auto i = 0u; i < message.parts(); ++i)
    {
        os << '[' << i << ']' << '(' << message.get(i) << ')';
    }

    os << '}';
}

inline
void traceSerializeImpl(std::ostream &os, const MDP::MessageHandle &handle)
{
    if(!handle) return;
    traceSerializeImpl(os, *handle);
}

}
