#pragma once

#include <zmqpp/zmqpp.hpp>

#include "ensure/Ensure.h"
#include "mdp/Except.h"
#include "mdp/MDP.h"

enum class IOMode
{
    NonBlockig,
    Blocking
};

MDP::MessageHandle recv(zmqpp::socket &, IOMode);
void send(zmqpp::socket &socket, MDP::Message, IOMode);
std::string asHex(const uint8_t *begin, const uint8_t *end);
