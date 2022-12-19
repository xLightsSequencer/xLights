/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "pch.h"

#include "../xLights/utils/string_utils.h"

TEST(String_Tests, IsHexChar_Test) {
    EXPECT_TRUE(isHexChar('0'));
    EXPECT_TRUE(isHexChar('1'));
    EXPECT_TRUE(isHexChar('2'));
    EXPECT_TRUE(isHexChar('3'));
    EXPECT_TRUE(isHexChar('4'));
    EXPECT_TRUE(isHexChar('5'));
    EXPECT_TRUE(isHexChar('6'));
    EXPECT_TRUE(isHexChar('7'));
    EXPECT_TRUE(isHexChar('8'));
    EXPECT_TRUE(isHexChar('9'));
    EXPECT_TRUE(isHexChar('a'));
    EXPECT_TRUE(isHexChar('b'));
    EXPECT_TRUE(isHexChar('c'));
    EXPECT_TRUE(isHexChar('d'));
    EXPECT_TRUE(isHexChar('e'));
    EXPECT_TRUE(isHexChar('f'));
}

TEST(String_Tests, IsNotHexChar_Test) {
    EXPECT_FALSE(isHexChar('g'));
    EXPECT_FALSE(isHexChar('z'));
    EXPECT_FALSE(isHexChar('s'));
    EXPECT_FALSE(isHexChar(' '));
    EXPECT_FALSE(isHexChar('-'));
}

TEST(String_Tests, CountChar_Test) {
    EXPECT_EQ(CountChar("rsrtrRr", 'r'), 4);
    EXPECT_EQ(CountChar("rsrtrRr", 'p'), 0);
}

TEST(String_Tests, CountStrings_Test) {
    EXPECT_EQ(CountStrings("rr", "rrsrrtrrRrr"), 4);
    EXPECT_EQ(CountStrings("rr", "rsrtrRr"), 0);
}

TEST(String_Tests, HexToChar_Test) {
    EXPECT_EQ(HexToChar('0'), 0x00);
    EXPECT_EQ(HexToChar('A'), 0x0A);
    EXPECT_EQ(HexToChar('F'), 0x0F);
}

TEST(String_Tests, HexToChar2_Test) {
    EXPECT_EQ(HexToChar('0', 'A'), (char)0x0A);
    EXPECT_EQ(HexToChar('A', 'F'), (char)0xAF);
}

