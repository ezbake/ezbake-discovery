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
 * ServiceDiscoveryClient.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: oarowojolu
 */

#include <ezbake/ezdiscovery/ServiceDiscoverySyncClient.h>
#include <boost/shared_ptr.hpp>
#include <sstream>

namespace ezbake { namespace ezdiscovery {

using namespace org::apache::zookeeper;


void ServiceDiscoverySyncClient::registerEndpoint(const ::std::string& serviceName,
        const ::std::string& point) {
    registerEndpoint(JUST_SERVICE_APP_NAME, serviceName, point);
}


void ServiceDiscoverySyncClient::registerEndpoint(const ::std::string& appName,
        const ::std::string& serviceName, const ::std::string& point) {
    validateHostAndPort(point); //validate the host and port for the point
    createPath(makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH, point));
}


void ServiceDiscoverySyncClient::unregisterEndpoint(const ::std::string& serviceName,
        const ::std::string& point) {
    unregisterEndpoint(JUST_SERVICE_APP_NAME, serviceName, point);
}


void ServiceDiscoverySyncClient::unregisterEndpoint(const ::std::string& appName,
        const ::std::string& serviceName, const ::std::string& point) {
    std::string path = makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH, point);

    //delete the endpoint
    ReturnCode::type response = _handle.remove(path, -1);

    if (response != ReturnCode::Ok && response != ReturnCode::NoNode) {
        THROW_EXCEPTION(ServiceDiscoveryException, "Error in unregistering endpoint: " + path);
    }
}


::std::vector< ::std::string> ServiceDiscoverySyncClient::getApplications() {
    return getChildren("");
}


::std::vector< ::std::string> ServiceDiscoverySyncClient::getServices() {
    return getServices(JUST_SERVICE_APP_NAME);
}


::std::vector< ::std::string> ServiceDiscoverySyncClient::getServices(const ::std::string& appName) {
    ::std::vector< ::std::string> services;
    ::std::string path = makeZKPath(appName);

    if (checkPathExists(path)) {
        return getChildren(path);
    }
    return services;
}


::std::vector< ::std::string> ServiceDiscoverySyncClient::getEndpoints(const ::std::string& serviceName) {
    return getEndpoints(JUST_SERVICE_APP_NAME, serviceName);
}


std::vector<std::string> ServiceDiscoverySyncClient::getEndpoints(const ::std::string& appName,
        const ::std::string& serviceName) {
    ::std::vector< ::std::string> endpoints;
    ::std::string path = makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH);

    if (checkPathExists(path)) {
        return getChildren(path);
    }
    return endpoints;
}


void ServiceDiscoverySyncClient::setSecurityIdForApplication(const ::std::string& applicationName,
        const ::std::string& securityId) {
    createPath(makeZKPath(applicationName,
            SECURITY_ZK_PATH, SECURITY_ID_NODE, securityId));
}


void ServiceDiscoverySyncClient::setSecurityIdForCommonService(const ::std::string& serviceName,
        const ::std::string& securityId) {
    createPath(makeZKPath(JUST_SERVICE_APP_NAME,
            serviceName, SECURITY_ZK_PATH, SECURITY_ID_NODE, securityId));
}


const std::string ServiceDiscoverySyncClient::getSecurityIdForApplication(const ::std::string& applicationName) {
    ::std::string path = makeZKPath(applicationName, SECURITY_ZK_PATH,
            SECURITY_ID_NODE);

    ::std::vector< ::std::string>nodes = getChildren(path);
    return (nodes.size() ? nodes.at(0) : "");
}


const ::std::string ServiceDiscoverySyncClient::getSecurityIdForCommonService(const ::std::string& serviceName) {
    ::std::string path = makeZKPath(JUST_SERVICE_APP_NAME, serviceName,
            SECURITY_ZK_PATH, SECURITY_ID_NODE);

    ::std::vector< ::std::string>nodes = getChildren(path);
    return (nodes.size() ? nodes.at(0) : "");
}


bool ServiceDiscoverySyncClient::isServiceCommon(const ::std::string& serviceName) {
    return checkPathExists(makeZKPath(JUST_SERVICE_APP_NAME, serviceName));
}


bool ServiceDiscoverySyncClient::checkPathExists(const ::std::string& path) {
    data::Stat stat;
    ReturnCode::type response = _handle.exists(path, boost::shared_ptr<Watch>(), stat);

    if (response != ReturnCode::Ok && response != ReturnCode::NoNode) {
        ::std::ostringstream ss;
        ss << "Error in checking path existence. ZK error: " << response;
        THROW_EXCEPTION(ServiceDiscoveryException, ss.str());
    }

    return (response != ReturnCode::NoNode);
}


void ServiceDiscoverySyncClient::createPath(const ::std::string& path) {
    /*
     * Don't handle response remove, since if it doesn't exists its all good,
     * as we are going to create it
     */
    _handle.remove(path, -1);

    ::std::string pathCreated;

    /*
     * Create parents. No need to check if they already exists because even if one doens't
     * another ZooKeeper user could create one after we've checked.
     */
    ::std::vector< ::std::string>paths = splitPath(path);
    ::std::string pathToCreate = "";
    for (unsigned int i = 0; i < (paths.size() - 1); i++) {
        pathToCreate += PATH_DELIM + paths.at(i);
        ReturnCode::type response = _handle.create(pathToCreate, "",
                                                   SD_DEFAULT_ACL, CreateMode::Persistent,
                                                   pathCreated);
        if (response != ReturnCode::Ok && response != ReturnCode::NodeExists) {
            ::std::ostringstream ss;
            ss << "Error in creating parents for node: " << pathToCreate << " ZK error: " << response;
            THROW_EXCEPTION(ServiceDiscoveryException, ss.str());
        }
    }

    //create child node
    ReturnCode::type response = _handle.create(path, "", SD_DEFAULT_ACL, CreateMode::Persistent, pathCreated);
    if (response != ReturnCode::Ok) {
        ::std::ostringstream ss;
        ss << "Error in creating node: " << path << " ZK error: " << response;
        THROW_EXCEPTION(ServiceDiscoveryException, ss.str());
    }
}

::std::vector< ::std::string> ServiceDiscoverySyncClient::getChildren(const ::std::string& path) {
    data::Stat stat;
    ::std::vector< ::std::string> children;

    ReturnCode::type response = _handle.getChildren(path, boost::shared_ptr<Watch>(), children, stat);

    if (response != ReturnCode::Ok && response != ReturnCode::NoNode) {
        ::std::ostringstream ss;
        ss << "Error in getting children. ZK error: " << response;
        THROW_EXCEPTION(ServiceDiscoveryException, ss.str());
    }

    return children;
}

}} // namespace ::ezbake::ezdiscovery
