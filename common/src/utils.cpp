#include <iterator>

#include "mdp/utils.h"

namespace {

char toGlyph(int c)
{
    static const char lookup[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    return 16 > c ? lookup[c] : '?';
}

} // namespace

MDP::MessageHandle recv(zmqpp::socket &socket, IOMode ioMode)
{
    auto handle       = MDP::makeMessageHandle();
    const auto status = socket.receive(
        *handle, IOMode::NonBlockig == ioMode /* dont_block */);

    if (status) return handle;
    return nullptr;
}

void send(zmqpp::socket &socket, MDP::Message message, IOMode ioMode)
{
    const auto status
        = socket.send(message, IOMode::NonBlockig == ioMode /* dont_block */);

    ENSURE(status, SendFailed);
}

std::string asHex(const uint8_t *begin, const uint8_t *const end)
{
    if (begin == end) return {};

    const auto len = std::distance(begin, end);

    std::string r((len << 1) + 1, '\0');

    for (auto i = 0u; i < len; ++i)
    {
        r[i * 2]     = toGlyph((begin[i] >> 4) & 0xF);
        r[i * 2 + 1] = toGlyph(begin[i] & 0xF);
    }
    return r;
}
