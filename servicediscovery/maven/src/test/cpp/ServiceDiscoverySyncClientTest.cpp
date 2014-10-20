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
 * ServiceDiscoverySyncClientTest.cpp
 *
 *  Created on: Mar 3, 2014
 *      Author: oarowojolu
 */

#include "contrib/gtest/gtest.h"
#include <ezbake/ezdiscovery/ServiceDiscoverySyncClient.h>
#include "../resources/ZKLocalTestServer.h"
#include <algorithm>
#include <boost/thread/thread.hpp>

namespace {

/**
 * Synchronous Service Discovery Test class
 */
class ServiceDiscoverySyncClientTest : public ::testing::Test {
public:
    ServiceDiscoverySyncClientTest() {}
    virtual ~ServiceDiscoverySyncClientTest() {}

    void SetUp() {
        ezbake::local::ZKLocalTestServer::start();

        std::ostringstream ss;
        ss << "localhost:" << ezbake::local::ZKLocalTestServer::DEFAULT_PORT;
        _client.init(ss.str());
    }

    void TearDown() {
        _client.close();
        ezbake::local::ZKLocalTestServer::stop();
        boost::this_thread::sleep(boost::posix_time::seconds(1)); //wait for 1 sec before starting next test
    }

protected:
    ezbake::ezdiscovery::ServiceDiscoverySyncClient _client;
};

TEST_F(ServiceDiscoverySyncClientTest, registionUnRegistration) {
    std::string appName = "seasme_street";
    std::string serviceName = "cookie_monster";
    std::vector<std::string> expectedEndpoints;
    expectedEndpoints.push_back("bigbird:2181");
    expectedEndpoints.push_back("elmo:2181");

    _client.registerEndpoint(appName, serviceName, expectedEndpoints[0]);
    _client.registerEndpoint(appName, serviceName, expectedEndpoints[1]);

    std::vector<std::string> endpoints = _client.getEndpoints(appName, serviceName);
    ASSERT_EQ(expectedEndpoints.size(), endpoints.size());
    std::sort(expectedEndpoints.begin(), expectedEndpoints.end());
    std::sort(endpoints.begin(), endpoints.end());
    EXPECT_EQ(expectedEndpoints, endpoints);

    _client.unregisterEndpoint(appName, serviceName, expectedEndpoints[0]);
    endpoints = _client.getEndpoints(appName, serviceName);
    EXPECT_EQ(static_cast<unsigned int>(1), endpoints.size());
    EXPECT_EQ(expectedEndpoints[1], endpoints[0]);

    _client.unregisterEndpoint(appName, serviceName, expectedEndpoints[1]);
    endpoints = _client.getEndpoints(appName, serviceName);
    EXPECT_EQ(static_cast<unsigned int>(0), endpoints.size());
}

TEST_F(ServiceDiscoverySyncClientTest, unregisteringEndpointsThatDoNotExist) {
    //ensure we do not throw exceptions when unregistering an non-exisiting node
    EXPECT_NO_THROW(_client.unregisterEndpoint("seasme_street", "cookie_monster", "does_not_exist:1234"));
}

TEST_F(ServiceDiscoverySyncClientTest, getApplications) {
    _client.registerEndpoint("App1", "service", "bigbird:2181");
    _client.registerEndpoint("App2", "service", "bigbird:2181");

    std::vector<std::string> apps = _client.getApplications();
    std::sort(apps.begin(), apps.end());

    ASSERT_EQ(static_cast<unsigned int>(2), apps.size());

    std::vector<std::string> expectedApps;
    expectedApps.push_back("App1");
    expectedApps.push_back("App2");
    EXPECT_EQ(expectedApps, apps);
}

TEST_F(ServiceDiscoverySyncClientTest, getServices) {
    std::string appName = "App3";

    _client.registerEndpoint(appName, "service1", "bigbird:2181");
    _client.registerEndpoint(appName, "service2", "bigbird:2181");

    std::vector<std::string> services = _client.getServices(appName);
    std::sort(services.begin(), services.end());

    ASSERT_EQ(static_cast<unsigned int>(2), services.size());

    std::vector<std::string> expectedServices;
    expectedServices.push_back("service1");
    expectedServices.push_back("service2");
    EXPECT_EQ(expectedServices, services);
}

TEST_F(ServiceDiscoverySyncClientTest, getApplicationsWithNoApplications) {
    std::vector<std::string> apps = _client.getApplications();
    EXPECT_EQ(static_cast<unsigned int>(0), apps.size());
}

TEST_F(ServiceDiscoverySyncClientTest, getServicesWithNotServicesRegistered) {
    std::vector<std::string> services = _client.getServices();
    EXPECT_EQ(static_cast<unsigned int>(0), services.size());
}

TEST_F(ServiceDiscoverySyncClientTest, getEndpointsWithNothingRegistered) {
    std::vector<std::string> endpoints = _client.getEndpoints("NONEXISTENT_SERVICE");
    EXPECT_EQ(static_cast<unsigned int>(0), endpoints.size());
}

TEST_F(ServiceDiscoverySyncClientTest, addForwardSlashInAppFirstChar) {
    std::string appName = "/app";
    std::string serviceName = "soup";

    _client.registerEndpoint(appName, serviceName, "bigbird:2181");

    std::vector<std::string> endpoints = _client.getEndpoints(appName, serviceName);
    ASSERT_EQ(static_cast<unsigned int>(1), endpoints.size());
    EXPECT_EQ("bigbird:2181", endpoints[0]);
}

TEST_F(ServiceDiscoverySyncClientTest, emptyServiceRegistration) {
    EXPECT_ANY_THROW(_client.registerEndpoint("", "bigbird:2181"));
}

TEST_F(ServiceDiscoverySyncClientTest, addSlashInService) {
    EXPECT_ANY_THROW(_client.registerEndpoint("hello/world", "bigbird:2181"));
}

TEST_F(ServiceDiscoverySyncClientTest, addSlashInApp) {
    EXPECT_ANY_THROW(_client.registerEndpoint("/ap/p", "soup", "bigbird:2181"));
}

TEST_F(ServiceDiscoverySyncClientTest, addSlashInEndpoint) {
    EXPECT_ANY_THROW(_client.registerEndpoint("/app", "soup", "bigbird://2181"));
}

TEST_F(ServiceDiscoverySyncClientTest, setGetSecurityIdForApplicationTest) {
    std::string appName = "abby_cadabby";
    std::string id = "ID0001";

    _client.setSecurityIdForApplication(appName, id);
    EXPECT_EQ(id, _client.getSecurityIdForApplication(appName));
    std::vector<std::string> apps = _client.getApplications();
    ASSERT_EQ(static_cast<unsigned int>(1), apps.size());
    EXPECT_EQ(appName, apps[0]);
}

TEST_F(ServiceDiscoverySyncClientTest, setGetSecurityIdForCommonService) {
    std::string serviceName = "telly_monster";
    std::string id = "ID0002";

    _client.setSecurityIdForCommonService(serviceName, id);
    EXPECT_EQ(id, _client.getSecurityIdForCommonService(serviceName));
    std::vector<std::string> services = _client.getServices();
    ASSERT_EQ(static_cast<unsigned int>(1), services.size());
    EXPECT_EQ(serviceName, services[0]);
}

TEST_F(ServiceDiscoverySyncClientTest, testBlankSecurityIdForApplilcation) {
    EXPECT_ANY_THROW(_client.setSecurityIdForApplication("abby_cadabby", ""));
}

TEST_F(ServiceDiscoverySyncClientTest, testBlankSecurityIdForCommonService) {
    EXPECT_ANY_THROW(_client.setSecurityIdForCommonService("telly_monster", ""));
}

TEST_F(ServiceDiscoverySyncClientTest, isServiceCommon) {
    std::string appName = "seasme_street";
    std::string serviceName = "cookie_monster";
    _client.registerEndpoint(appName, serviceName, "bigbird:2182");
    _client.registerEndpoint(appName, serviceName, "elmo:2182");
    EXPECT_FALSE(_client.isServiceCommon(serviceName));

    serviceName = "telly_monster";
    _client.registerEndpoint(serviceName, "bigbird:2182");
    _client.registerEndpoint(serviceName, "elmo:2182");
    EXPECT_TRUE(_client.isServiceCommon(serviceName));
}

} //namespace
