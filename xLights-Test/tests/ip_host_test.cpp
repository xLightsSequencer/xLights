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

#include "wxfixture.h"

#include "../xLights/utils/ip_utils.h"

TEST_F(IP_Host_Tests, IP_Valid) {
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("192.168.1.1"));
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("192.168.1.11"));
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("192.168.1.111"));
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("10.10.10.10"));
}

TEST_F(IP_Host_Tests, IP_Invalid) {
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("192.168.1111.1"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("192.168..1"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("192.168.111."));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("192.168.111"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("256.168.111.1"));
}

TEST_F(IP_Host_Tests, Host_Valid) {
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("fpp.local"));
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("fpptest.local"));
    EXPECT_TRUE(ip_utils::IsIPValidOrHostname("localhost"));
}

TEST_F(IP_Host_Tests, Host_Invalid) {
    //need to fix
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("testing testing"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("-fpp.local"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("fpp.local-"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname("fpptest.l"));
    EXPECT_FALSE(ip_utils::IsIPValidOrHostname(".local"));
}