#include "mdp/MutualHeartbeatMonitor.h"

std::chrono::milliseconds MutualHeartbeatMonitor::peerDiff() const
{
    const auto diff = std::chrono::steady_clock::now() - peerTimestamp_;
    const auto msDiff
        = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    return msDiff;
}

std::chrono::milliseconds MutualHeartbeatMonitor::selfDiff() const
{
    const auto diff = std::chrono::steady_clock::now() - selfTimestamp_;
    const auto msDiff
        = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
    return msDiff;
}

MutualHeartbeatMonitor::MutualHeartbeatMonitor(std::chrono::milliseconds period)
    : period_{period}
    , selfTimestamp_{std::chrono::steady_clock::now()}
    , peerTimestamp_{selfTimestamp_}
{ }

std::chrono::milliseconds MutualHeartbeatMonitor::period() const
{
    return period_;
}
bool MutualHeartbeatMonitor::peerHeartbeatExpired() const
{
    return peerDiff() > 3 * period();
}
bool MutualHeartbeatMonitor::shouldHeartbeat() const
{
    return selfDiff() > period();
}
void MutualHeartbeatMonitor::selfHeartbeat()
{
    selfTimestamp_ = std::chrono::steady_clock::now();
}
void MutualHeartbeatMonitor::peerHeartbeat()
{
    peerTimestamp_ = std::chrono::steady_clock::now();
}

void MutualHeartbeatMonitor::reset()
{
    const auto now = std::chrono::steady_clock::now();
    selfTimestamp_ = now;
    peerTimestamp_ = now;
}

void MutualHeartbeatMonitor::stats(std::ostream &os) const
{
    os << "selfDiff " << selfDiff().count() << "ms"
       << " peerDiff " << peerDiff().count() << "ms";
}

std::ostream &
operator<<(std::ostream &os, const MutualHeartbeatMonitor &monitor)
{
    monitor.stats(os);
    return os;
}
