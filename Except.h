#pragma once

#include "Ensure.h"


/* TODO: support proper customization */
using MessageInvalid = Exception<std::invalid_argument>;
using MessageFormatInvalid = Exception<std::invalid_argument>;
using BrokerMessageFormatInvalid = MessageFormatInvalid;
using WorkerMessageFormatInvalid = MessageFormatInvalid;
using ClientMessageFormatInvalid = MessageFormatInvalid;
using WorkerMessageFormatInvalid = MessageFormatInvalid;

using BrokerHeartbeatExpired = Exception<std::runtime_error>;
using BrokerDisconnected = Exception<std::runtime_error>;

using WorkerMessageFormatInvalid = MessageFormatInvalid;

using FlowError = Exception<std::runtime_error>;

using RecvFailed = Exception<std::runtime_error>;
using SendFailed = Exception<std::runtime_error>;

using WorkerDuplicate = Exception<std::runtime_error>;
using ServiceUnsupported = Exception<std::runtime_error>;

using IdentityInvalid = Exception<std::invalid_argument>;
