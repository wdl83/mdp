#include <map>

#include <gtest/gtest.h>

#include "mdp/ZMQIdentity.h"


TEST(ZMQIdentityTest, Custom)
{
    ZMQIdentity id("test");

    ASSERT_EQ(id.asString(), "test");
    ASSERT_TRUE(static_cast<bool>(id));

}

TEST(ZMQIdentityTest, Empty)
{
    ZMQIdentity empty;

    ASSERT_FALSE(static_cast<bool>(empty));
    ASSERT_EQ(empty.asString(), "");
}

TEST(ZMQIdentityTest, Unique)
{
    ZMQIdentity id1 = ZMQIdentity::unique();
    ZMQIdentity id2 = ZMQIdentity::unique();

    ASSERT_GT(id1.size(), 0);
    ASSERT_GT(id2.size(), 0);
    ASSERT_NE(id1, id2);
    ASSERT_NE(id1.asString(), id2.asString());
}

TEST(ZMQIdentityTest, Compare)
{
    ZMQIdentity id1("a");
    ZMQIdentity id2("b");
    ZMQIdentity id3("a");

    ASSERT_LT(id1, id2);
    ASSERT_EQ(id1, id3);
    ASSERT_NE(id1, id2);
}

TEST(ZMQIdentityTest, Uniqueness)
{
    std::set<ZMQIdentity> all;

    for (int i = 0; i < 1000; ++i)
        ASSERT_TRUE(all.insert(ZMQIdentity::unique()).second);
}
