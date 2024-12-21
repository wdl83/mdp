#include "Ensure.h"
#include "sys/types.h"

#include <limits.h>

#define _GNU_SOURCE
#include "unistd.h"
#undef _GNU_SOURCE

#include <mutex>
#include <sstream>
#include <thread>

#include "ZMQIdentity.h"


thread_local int ZMQIdentity::no_;

namespace
{
std::once_flag onceFlag;
char hostname[HOST_NAME_MAX];
} // namespace

std::string ZMQIdentity::uniqueId()
{
    std::call_once(
        onceFlag,
        []() { ENSURE(0 == ::gethostname(hostname, sizeof(hostname)), RuntimeError); });

    const auto id =
        hostname
        + std::to_string(::getpid())
        + ':' + std::to_string(::gettid())
        + '@' + std::to_string(++no_);
    return id;
}
