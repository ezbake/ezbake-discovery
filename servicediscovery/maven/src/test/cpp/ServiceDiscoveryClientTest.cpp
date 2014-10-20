/*   Copyright (C) 2013-2014 Computer Sciences Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

/*
 * ServiceDiscoveryClientTest.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: oarowojolu
 */

#include "contrib/gtest/gtest.h"
#include <boost/thread/thread.hpp>
#include "../resources/ZKLocalTestServer.h"
#include <ezbake/ezdiscovery/ServiceDiscoveryClient.h>

namespace {

/**
 * Generic Service Discovery Test class
 */
class ServiceDiscoveryClientTest : public ::testing::Test {
public:
    ServiceDiscoveryClientTest() {}
    virtual ~ServiceDiscoveryClientTest() {}

    void SetUp() {
        ezbake::local::ZKLocalTestServer::start();
    }

    void TearDown() {
        ezbake::local::ZKLocalTestServer::stop();
        boost::this_thread::sleep(boost::posix_time::seconds(1)); //wait for 1 sec before starting next test
    }
};

TEST_F(ServiceDiscoveryClientTest, DummyTest) {
    ASSERT_TRUE(true);
}

TEST_F(ServiceDiscoveryClientTest, TestInit) {
    std::ostringstream ss;
    ss << "localhost:" << ezbake::local::ZKLocalTestServer::DEFAULT_PORT;

    ezbake::ezdiscovery::ServiceDiscoveryClient client;
    client.init(ss.str());
    client.close();
}

TEST_F(ServiceDiscoveryClientTest, MakePathAndSplitPath) {
    std::vector<std::string> paths;
    paths.push_back("No");
    paths.push_back(".");
    paths.push_back("I");
    paths.push_back("am");
    paths.push_back("the");
    paths.push_back("one");
    paths.push_back("who");
    paths.push_back("knocks!");
    std::string expectedPath = "/No/./I/am/the/one/who/knocks!";

    std::string path = ezbake::ezdiscovery::ServiceDiscoveryClient::makeZKPath(paths[0],
            paths[1], paths[2], paths[3], paths[4], paths[5], paths[6], paths[7]);
    EXPECT_EQ(expectedPath, path);

    EXPECT_EQ(paths, ezbake::ezdiscovery::ServiceDiscoveryClient::splitPath(expectedPath));
}

TEST_F(ServiceDiscoveryClientTest, MakePathWithBlankInBetween) {
    std::string path = ezbake::ezdiscovery::ServiceDiscoveryClient::makeZKPath("Hello", "", "World");
    EXPECT_EQ("/Hello//World", path);
}

TEST_F(ServiceDiscoveryClientTest, ValidateHostAndPort) {
    EXPECT_NO_THROW(ezbake::ezdiscovery::ServiceDiscoveryClient::validateHostAndPort("localhost:122"));
    EXPECT_ANY_THROW(ezbake::ezdiscovery::ServiceDiscoveryClient::validateHostAndPort("www.failtest.com"));
}

} //namespace
