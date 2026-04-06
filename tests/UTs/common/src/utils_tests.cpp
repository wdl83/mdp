#include <gtest/gtest.h>

#include "mdp/utils.h"


TEST(UtilsTest, AsHex)
{
    const uint8_t *emptyBegin = nullptr;
    const uint8_t *emptyEnd = nullptr;
    EXPECT_STREQ(asHex(emptyBegin, emptyEnd).c_str(), "");

    uint8_t singleByte[] = {0x0A};
    EXPECT_STREQ(asHex(singleByte, singleByte + 1).c_str(), "0A");

    uint8_t multipleBytes[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    EXPECT_STREQ(asHex(multipleBytes, multipleBytes + 8).c_str(), "0123456789ABCDEF");

    uint8_t allZeros[] = {0x00, 0x00, 0x00};
    EXPECT_STREQ(asHex(allZeros, allZeros + 3).c_str(), "000000");

    uint8_t allOnes[] = {0xFF, 0xFF, 0xFF};
    EXPECT_STREQ(asHex(allOnes, allOnes + 3).c_str(), "FFFFFF");

    uint8_t mixedCase[] = {0x12, 0xab, 0x34, 0xCD, 0x56, 0xef};
    EXPECT_STREQ(asHex(mixedCase, mixedCase + 6).c_str(), "12AB34CD56EF");

    uint8_t seq256[256];
    std::string expected256;

    for (int i = 0; i < 256; ++i)
    {
        seq256[i] = static_cast<uint8_t>(i);

        char buf[3];
        sprintf(buf, "%02X", i);
        expected256 += buf;
    }

    EXPECT_STREQ(asHex(seq256, seq256 + 256).c_str(), expected256.c_str());
}
