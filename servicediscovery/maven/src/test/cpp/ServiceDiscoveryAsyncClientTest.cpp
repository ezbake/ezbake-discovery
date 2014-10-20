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
 * ServiceDiscoveryAsyncClientTest.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: oarowojolu
 */



#include "contrib/gtest/gtest.h"
#include <ezbake/ezdiscovery/ServiceDiscoveryAsyncClient.h>
#include <ezbake/ezdiscovery/ServiceDiscoverySyncClient.h>
#include "../resources/ZKLocalTestServer.h"
#include <algorithm>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>

namespace {

/*
 * Class to control our wait for asynchronous callbacks for our test cases
 */
class AsyncCallbackWait {
public:
    AsyncCallbackWait() : _completed(true) {}

    void notifyCompleted() {
        {
            boost::lock_guard<boost::mutex> lock(_mutex);
            _completed = true;
        }
        _cond.notify_all();
    }

    void waitForCompleted() {
        _completed = false;
        boost::unique_lock<boost::mutex> lock(_mutex);
        while (!_completed) {
            _cond.wait(lock);
        }
    }

private:
    boost::condition_variable _cond;
    boost::mutex _mutex;
    bool _completed;
};

/**
 * Asynchronous Service Discovery Test class
 */
class ServiceDiscoveryAsyncClientTest : public ::testing::Test {
public:
    ServiceDiscoveryAsyncClientTest() {}
    virtual ~ServiceDiscoveryAsyncClientTest() {}

    void SetUp() {
        ezbake::local::ZKLocalTestServer::start();

        std::ostringstream ss;
        ss << "localhost:" << ezbake::local::ZKLocalTestServer::DEFAULT_PORT;
        connectString = ss.str();

        _client.init(connectString);
    }

    void TearDown() {
        _client.close();
        ezbake::local::ZKLocalTestServer::stop();
        boost::this_thread::sleep(boost::posix_time::seconds(1)); //wait for 1 sec before starting next test
    }

protected:
    std::string connectString;
    ezbake::ezdiscovery::ServiceDiscoveryAsyncClient _client;
    static AsyncCallbackWait _callbackWait;


protected:
    /*
     * Declare our test callbacks
     */

    class OperationCallback : public ezbake::ezdiscovery::ServiceDiscoveryOpCallback {
    public:
        OperationCallback(bool& opResponse) : _opResponse(opResponse) {}

        virtual void process(CallbackResponse response) {
            _opResponse = (response == ServiceDiscoveryCallback::OK);
            _callbackWait.notifyCompleted();
        }

    private:
        bool& _opResponse;
    };

    class StatusCallback : public ezbake::ezdiscovery::ServiceDiscoveryStatusCallback {
    public:
        StatusCallback(bool& opResponse, bool& status) : _opResponse(opResponse), _status(status) {}

        virtual void process(CallbackResponse response, bool status) {
            _opResponse = (response == ServiceDiscoveryCallback::OK);
            _status = status;
            _callbackWait.notifyCompleted();
        }

    private:
        bool& _opResponse;
        bool& _status;
    };

    class NodeCallback : public ezbake::ezdiscovery::ServiceDiscoveryNodeCallback {
    public:
        NodeCallback(bool& opResponse, std::string& node) : _opResponse(opResponse), _node(node) {}

        virtual void process(CallbackResponse response, const std::string& value) {
            _opResponse = (response == ServiceDiscoveryCallback::OK);
            _node = value;
            _callbackWait.notifyCompleted();
        }

    private:
        bool& _opResponse;
        std::string& _node;
    };

    class ListCallback : public ezbake::ezdiscovery::ServiceDiscoveryListCallback {
    public:
        ListCallback(bool& response, std::vector<std::string>& nodes) : _opResponse(response), _nodes(nodes) {}

        virtual void process(CallbackResponse response, const std::vector<std::string>& values) {
            _opResponse = (response == ServiceDiscoveryCallback::OK);
            _nodes = values;
            _callbackWait.notifyCompleted();
        }

    private:
        bool& _opResponse;
        std::vector<std::string>& _nodes;
    };

};

//Declare the static callback
AsyncCallbackWait ServiceDiscoveryAsyncClientTest::_callbackWait;


TEST_F(ServiceDiscoveryAsyncClientTest, registionUnRegistration) {
    std::string appName = "seasme_street";
    std::string serviceName = "cookie_monster";
    std::vector<std::string> expectedEndpoints;
    expectedEndpoints.push_back("bigbird:2181");
    expectedEndpoints.push_back("elmo:2181");
    bool callbackResponse = false;

    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    _client.registerEndpoint(appName, serviceName, expectedEndpoints[0], registerCB);

    //wait for operation to complete and callback to be executed ...
    _callbackWait.waitForCompleted();

    //callback has executed, check results
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.registerEndpoint(appName, serviceName, expectedEndpoints[1], registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);


    callbackResponse = false;
    std::vector<std::string> endpoints;
    boost::shared_ptr<ListCallback> getEndpointsCB(new ListCallback(callbackResponse, endpoints));

    _client.getEndpoints(appName, serviceName, getEndpointsCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    ASSERT_EQ(expectedEndpoints.size(), endpoints.size());
    std::sort(expectedEndpoints.begin(), expectedEndpoints.end());
    std::sort(endpoints.begin(), endpoints.end());
    EXPECT_EQ(expectedEndpoints, endpoints);


    callbackResponse = false;
    boost::shared_ptr<OperationCallback> unregisterEndpointCB(new OperationCallback(callbackResponse));
    _client.unregisterEndpoint(appName, serviceName, expectedEndpoints[0], unregisterEndpointCB);

    //wait for operation to complete and callback to be executed ...
    _callbackWait.waitForCompleted();

    //callback has executed, check results
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.getEndpoints(appName, serviceName, getEndpointsCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(static_cast<unsigned int>(1), endpoints.size());
    EXPECT_EQ(expectedEndpoints[1], endpoints[0]);

    callbackResponse = false;
    _client.unregisterEndpoint(appName, serviceName, expectedEndpoints[1], unregisterEndpointCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    callbackResponse = false;
    _client.getEndpoints(appName, serviceName, getEndpointsCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(static_cast<unsigned int>(0), endpoints.size());
}

TEST_F(ServiceDiscoveryAsyncClientTest, unregisteringEndpointsThatDoNotExist) {
    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> callback(new OperationCallback(callbackResponse));

    //ensure we do not get an error when unregistering an non-exisiting node
    _client.unregisterEndpoint("seasme_street", "cookie_monster", "does_not_exist:1234", callback);

    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
}

TEST_F(ServiceDiscoveryAsyncClientTest, getApplications) {
    bool callbackResponse = false;

    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    _client.registerEndpoint("App1", "service", "bigbird:2181", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.registerEndpoint("App2", "service", "bigbird:2181", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);


    callbackResponse = false;
    std::vector<std::string> apps;
    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, apps));

    _client.getApplications(callback);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    ASSERT_EQ(static_cast<unsigned int>(2), apps.size());
    std::sort(apps.begin(), apps.end());
    std::vector<std::string> expectedApps;
    expectedApps.push_back("App1");
    expectedApps.push_back("App2");
    EXPECT_EQ(expectedApps, apps);
}

TEST_F(ServiceDiscoveryAsyncClientTest, getServices) {
    std::string appName = "App3";
    bool callbackResponse;

    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));

    callbackResponse = false;
    _client.registerEndpoint(appName, "service1", "bigbird:2181", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.registerEndpoint(appName, "service2", "bigbird:2181", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);


    callbackResponse = false;
    std::vector<std::string> services;
    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, services));

    _client.getServices(appName, callback);
    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
    ASSERT_EQ(static_cast<unsigned int>(2), services.size());

    std::sort(services.begin(), services.end());
    std::vector<std::string> expectedServices;
    expectedServices.push_back("service1");
    expectedServices.push_back("service2");
    EXPECT_EQ(expectedServices, services);
}

TEST_F(ServiceDiscoveryAsyncClientTest, getApplicationsWithNoApplications) {
    bool callbackResponse = false;
    std::vector<std::string> apps;
    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, apps));

    _client.getApplications(callback);
    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(static_cast<unsigned int>(0), apps.size());
}

TEST_F(ServiceDiscoveryAsyncClientTest, getServicesWithNotServicesRegistered) {
    bool callbackResponse = false;
    std::vector<std::string> services;
    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, services));

    _client.getServices(callback);
    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(static_cast<unsigned int>(0), services.size());
}

TEST_F(ServiceDiscoveryAsyncClientTest, getEndpointsWithNothingRegistered) {
    bool callbackResponse = false;
    std::vector<std::string> endpoints;
    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, endpoints));

    _client.getEndpoints("NONEXISTENT_SERVICE", callback);
    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(static_cast<unsigned int>(0), endpoints.size());
}

TEST_F(ServiceDiscoveryAsyncClientTest, addForwardSlashInAppFirstChar) {
    std::string appName = "/app";
    std::string serviceName = "soup";
    std::vector<std::string> endpoints;

    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    _client.registerEndpoint(appName, serviceName, "bigbird:2181", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    boost::shared_ptr<ListCallback> callback(new ListCallback(callbackResponse, endpoints));

    callbackResponse = false;
    _client.getEndpoints(appName, serviceName, callback);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    ASSERT_EQ(static_cast<unsigned int>(1), endpoints.size());
    EXPECT_EQ("bigbird:2181", endpoints[0]);
}

TEST_F(ServiceDiscoveryAsyncClientTest, emptyServiceRegistration) {
    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    EXPECT_ANY_THROW(_client.registerEndpoint("", "bigbird:2181", registerCB));
    //_callbackWait.waitForCompleted();
    //EXPECT_FALSE(callbackResponse);
}

TEST_F(ServiceDiscoveryAsyncClientTest, addSlashInService) {
    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    EXPECT_ANY_THROW(_client.registerEndpoint("hello/world", "bigbird:2181", registerCB));
}

TEST_F(ServiceDiscoveryAsyncClientTest, addSlashInApp) {
    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    EXPECT_ANY_THROW(_client.registerEndpoint("/ap/p", "soup", "bigbird:2181", registerCB));
}

TEST_F(ServiceDiscoveryAsyncClientTest, addSlashInEndpoint) {
    bool callbackResponse = false;
    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));
    EXPECT_ANY_THROW(_client.registerEndpoint("/app", "soup", "bigbird://2181", registerCB));
}

TEST_F(ServiceDiscoveryAsyncClientTest, setGetSecurityIdForApplicationTest) {
    std::string appName = "abby_cadabby";
    std::string id1 = "ID0001";
    bool callbackResponse;

    boost::shared_ptr<OperationCallback> callback(new OperationCallback(callbackResponse));

    callbackResponse = false;
    _client.setSecurityIdForApplication(appName, id1, callback);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    std::string node;
    boost::shared_ptr<NodeCallback> getSecurityIdCB(new NodeCallback(callbackResponse, node));

    callbackResponse = false;
    _client.getSecurityIdForApplication(appName, getSecurityIdCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(id1, node);
}

TEST_F(ServiceDiscoveryAsyncClientTest, setGetSecurityIdForCommonService) {
    std::string serviceName = "telly_monster";
    std::string id2 = "ID0002";
    bool callbackResponse;

    boost::shared_ptr<OperationCallback> callback(new OperationCallback(callbackResponse));

    callbackResponse = false;
    _client.setSecurityIdForCommonService(serviceName, id2, callback);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);


    std::string node;
    boost::shared_ptr<NodeCallback> getSecurityIdCB(new NodeCallback(callbackResponse, node));

    callbackResponse = false;
    _client.getSecurityIdForCommonService(serviceName, getSecurityIdCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    EXPECT_EQ(id2, node);
}

TEST_F(ServiceDiscoveryAsyncClientTest, isServiceCommon) {
    bool callbackResponse;
    std::string appName = "seasme_street";
    std::string serviceName = "cookie_monster";


    boost::shared_ptr<OperationCallback> registerCB(new OperationCallback(callbackResponse));

    callbackResponse = false;
    _client.registerEndpoint(appName, serviceName, "bigbird:2182", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.registerEndpoint(appName, serviceName, "elmo:2182", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);


    bool status = false;
    callbackResponse = false;
    boost::shared_ptr<StatusCallback> callback(new StatusCallback(callbackResponse, status));

    _client.isServiceCommon(serviceName, callback);
    _callbackWait.waitForCompleted();

    ASSERT_TRUE(callbackResponse);
    EXPECT_FALSE(status);


    callbackResponse = false;
    status = false;
    serviceName = "telly_monster";


    callbackResponse = false;
    _client.registerEndpoint(serviceName, "bigbird:2182", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.registerEndpoint(serviceName, "elmo:2182", registerCB);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);

    callbackResponse = false;
    _client.isServiceCommon(serviceName, callback);
    _callbackWait.waitForCompleted();
    ASSERT_TRUE(callbackResponse);
    EXPECT_TRUE(status);
}

} //namespace
