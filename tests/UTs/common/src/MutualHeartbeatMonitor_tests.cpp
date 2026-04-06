#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "mdp/MutualHeartbeatMonitor.h"

using namespace std::chrono;

TEST(MutualHeartbeatMonitorTests, ShouldHeartbeat)
{
    MutualHeartbeatMonitor monitor{milliseconds{100}};

    ASSERT_FALSE(monitor.peerHeartbeatExpired());

    std::this_thread::sleep_for(milliseconds(1));

    ASSERT_FALSE(monitor.shouldHeartbeat());

    std::this_thread::sleep_for(monitor.period());

    ASSERT_TRUE(monitor.shouldHeartbeat());
}

TEST(MutualHeartbeatMonitorTests, SelfHeartbeatReset)
{
    MutualHeartbeatMonitor monitor{milliseconds{50}};

    std::this_thread::sleep_for(monitor.period() + milliseconds(10));

    ASSERT_TRUE(monitor.shouldHeartbeat());

    monitor.selfHeartbeat();

    ASSERT_FALSE(monitor.shouldHeartbeat());
}

TEST(MutualHeartbeatMonitorTests, PeerHeartbeatReset)
{
    MutualHeartbeatMonitor monitor{milliseconds{70}};

    std::this_thread::sleep_for(3 * monitor.period() + milliseconds{1});

    ASSERT_TRUE(monitor.peerHeartbeatExpired());

    monitor.peerHeartbeat();

    ASSERT_FALSE(monitor.peerHeartbeatExpired());
}

TEST(MutualHeartbeatMonitorTests, Reset)
{
    MutualHeartbeatMonitor monitor{milliseconds{100}};

    std::this_thread::sleep_for(monitor.period() + milliseconds{1});

    ASSERT_TRUE(monitor.shouldHeartbeat());

    monitor.selfHeartbeat();

    ASSERT_FALSE(monitor.shouldHeartbeat());

    std::this_thread::sleep_for(2 * monitor.period() + milliseconds{1});

    ASSERT_TRUE(monitor.peerHeartbeatExpired());

    monitor.peerHeartbeat();

    ASSERT_FALSE(monitor.peerHeartbeatExpired());

    monitor.reset();

    ASSERT_FALSE(monitor.peerHeartbeatExpired());
    ASSERT_FALSE(monitor.shouldHeartbeat());

    std::this_thread::sleep_for(3 * monitor.period() + milliseconds{10});

    ASSERT_TRUE(monitor.peerHeartbeatExpired());
    ASSERT_TRUE(monitor.shouldHeartbeat());
}
