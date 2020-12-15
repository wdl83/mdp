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
constexpr auto self = "MDPC01";
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
constexpr auto self = "MDPW01";
constexpr auto ready = "\x1";
constexpr auto request = "\x2";
constexpr auto reply = "\x3";
constexpr auto heartbeat = "\x4";
constexpr auto disconnect = "\x5";
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
constexpr auto serviceUndefined = "error: service undefined";
constexpr auto serviceUnsupported = "service unsupported";
constexpr auto serviceBusy = "service busy";
constexpr auto serviceRegistered = "service registered";
constexpr auto serviceFailure =  "service failure";
constexpr auto statusSucess = "sucess";
constexpr auto statusFailure = "failure";
}

/* Client REPLY:
 *  Frame 0: Identity
 *  Frame 1: Empty (zero bytes, invisible to REQ application)
 *  Frame 2: "MDPC01" (six bytes, representing MDP/Client v0.1)
 *  Frame 3: Service name (printable string)
 *  Frame 4: status (sucess | failure)
 *  Frames 5+: Reply body (opaque binary) */
template <typename ...T_n>
Message makeSucessClientRep(
    const ZMQIdentity &identity,
    const std::string &service,
    const T_n & ...body)
{
    return makeMessage(
        identity,
        EmptyFrame{},
        Client::Signature::self,
        service,
        Broker::Signature::statusSucess,
        body...);
}

template <typename ...T_n>
Message makeFailureClientRep(
    const ZMQIdentity &identity,
    const std::string &service,
    const T_n & ...body)
{
    return makeMessage(
        identity,
        EmptyFrame{},
        Client::Signature::self,
        service,
        Broker::Signature::statusFailure,
        body...);
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


template <typename T>
void traceSerializeImpl(std::ostream &os, T value)
{
    os << value;
}

inline
void traceSerializeImpl(std::ostream &os, char value)
{
    os << int(value);
}

inline
void traceSerializeImpl(std::ostream &os, unsigned char value)
{
    os << int(value);
}

inline
void traceSerializeImpl(std::ostream &os, signed char value)
{
    os << int(value);
}

inline
void traceSerializeImpl(std::ostream &os, const MDP::Message &message)
{
    os << '{';

    for(auto i = 0u; i < message.parts(); ++i)
    {
        os << '[' << i << ']' << '(';
        traceSerializeImpl(os, message.get(i));
        os << ')';
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
