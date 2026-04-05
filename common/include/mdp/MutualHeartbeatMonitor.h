#pragma once

#include <chrono>
#include <ostream>


struct MutualHeartbeatMonitor
{
    static
    constexpr std::chrono::milliseconds period{3 * 1000};
private:
    std::chrono::steady_clock::time_point selfTimestamp;
    std::chrono::steady_clock::time_point peerTimestamp;

    std::chrono::milliseconds peerDiff() const
    {
        const auto diff = std::chrono::steady_clock::now() - peerTimestamp;
        const auto msDiff = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        return msDiff;
    }

    std::chrono::milliseconds selfDiff() const
    {
        const auto diff = std::chrono::steady_clock::now() - selfTimestamp;
        const auto msDiff = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        return msDiff;
    }
public:
    MutualHeartbeatMonitor(
        std::chrono::steady_clock::time_point tp = std::chrono::steady_clock::now()):
        selfTimestamp{tp},
        peerTimestamp{tp}
    {}

    bool peerHeartbeatExpired() const {return peerDiff() > 3 * period;}
    bool shouldHeartbeat() const {return selfDiff() > period;}
    void selfHeartbeat() {selfTimestamp = std::chrono::steady_clock::now();}
    void peerHeartbeat() {peerTimestamp = std::chrono::steady_clock::now();}

    void reset()
    {
        const auto now = std::chrono::steady_clock::now();
        selfTimestamp = now;
        peerTimestamp = now;
    }

    void stats(std::ostream &os) const
    {
        os
            << "selfDiff " << selfDiff().count() << "ms"
            << " peerDiff " << peerDiff().count() << "ms";
    }

    friend
    std::ostream &operator<< (std::ostream &os, const MutualHeartbeatMonitor &monitor)
    {
        monitor.stats(os);
        return os;
    }
};
