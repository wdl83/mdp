#pragma once

#include <zmqpp/zmqpp.hpp>

#include "Ensure.h"
#include "Except.h"


enum class IOMode
{
    NonBlockig,
    Blocking
};

inline
MDP::MessageHandle recv(zmqpp::socket &socket, IOMode ioMode)
{
    auto handle = MDP::makeMessageHandle();
    const auto status =
        socket.receive(*handle, IOMode::NonBlockig == ioMode /* dont_block */);

    if(status) return handle;
    return nullptr;
}

inline
void send(zmqpp::socket &socket, MDP::Message message, IOMode ioMode)
{
    const auto status =
        socket.send(message, IOMode::NonBlockig == ioMode /* dont_block */);

    ENSURE(status, SendFailed);
}

inline
char toGlyph(int c)
{
    static const char lookup[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    return 16 > c ? lookup[c] : '?';
}

inline
std::string asHex(const std::string &str)
{
    if(str.empty()) return {};

    std::string r((str.size() << 1) + 1, '\0');

    for(auto i = 0u; i < str.size(); ++i)
    {
        r[i * 2] = toGlyph((str[i] >> 4) & 0xF);
        r[i * 2 + 1] = toGlyph(str[i] & 0xF);
    }
    return r;
}
