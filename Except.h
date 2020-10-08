#pragma once

#include "Ensure.h"


/* TODO: support proper customization */
using MessageInvalid = EXCEPTION(std::invalid_argument);
using MessageFormatInvalid = EXCEPTION(std::invalid_argument);
using BrokerMessageFormatInvalid = EXCEPTION(std::invalid_argument);
using WorkerMessageFormatInvalid = EXCEPTION(std::invalid_argument);
using ClientMessageFormatInvalid = EXCEPTION(std::invalid_argument);

using BrokerHeartbeatExpired = EXCEPTION(std::runtime_error);
using BrokerDisconnected = EXCEPTION(std::runtime_error);

using FlowError = EXCEPTION(std::runtime_error);

using RecvFailed = EXCEPTION(std::runtime_error);
using SendFailed = EXCEPTION(std::runtime_error);

using WorkerDuplicate = EXCEPTION(std::runtime_error);
using ServiceUnsupported = EXCEPTION(std::runtime_error);

using IdentityInvalid = EXCEPTION(std::invalid_argument);
