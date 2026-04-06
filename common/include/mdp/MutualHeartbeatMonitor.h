#pragma once

#include <chrono>
#include <ostream>

class MutualHeartbeatMonitor
{
    std::chrono::milliseconds period_;
    std::chrono::steady_clock::time_point selfTimestamp_;
    std::chrono::steady_clock::time_point peerTimestamp_;

    std::chrono::milliseconds peerDiff() const;
    std::chrono::milliseconds selfDiff() const;
public:
    explicit MutualHeartbeatMonitor(
        std::chrono::milliseconds period = std::chrono::seconds{3});

    std::chrono::milliseconds period() const;
    bool peerHeartbeatExpired() const;
    bool shouldHeartbeat() const;
    void selfHeartbeat();
    void peerHeartbeat();
    void reset();
    void stats(std::ostream &os) const;

    friend std::ostream &
    operator<<(std::ostream &os, const MutualHeartbeatMonitor &monitor);
};
