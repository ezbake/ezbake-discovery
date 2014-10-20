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
 * ServiceDiscoveryUtils.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: oarowojolu
 */

#include <ezbake/ezdiscovery/ServiceDiscoveryClient.h>

namespace ezbake { namespace ezdiscovery {

using namespace org::apache::zookeeper;

const ::std::string ServiceDiscoveryClient::NAMESPACE = "ezDiscovery";
const ::std::string ServiceDiscoveryClient::ENDPOINTS_ZK_PATH = "endpoints";
const ::std::string ServiceDiscoveryClient::JUST_SERVICE_APP_NAME = "common_services";
const ::std::string ServiceDiscoveryClient::SECURITY_ZK_PATH = "security";
const ::std::string ServiceDiscoveryClient::SECURITY_ID_NODE = "security_id";
const ::std::string ServiceDiscoveryClient::PATH_DELIM = "/";
const SDACL ServiceDiscoveryClient::OPEN_ACL_UNSAFE_ACL = SDACL("world", "anyone", Permission::All);
const SDACL ServiceDiscoveryClient::READ_ACL_UNSAFE_ACL = SDACL("world", "anyone", Permission::Read);
const SDACL ServiceDiscoveryClient::CREATOR_ALL_ACL_ACL = SDACL("auth", "", Permission::All);
const ::std::vector<data::ACL> ServiceDiscoveryClient::SD_DEFAULT_ACL = ::std::vector<data::ACL>(1, OPEN_ACL_UNSAFE_ACL);


void ServiceDiscoveryClient::close() {
    _handle.close();
}


void ServiceDiscoveryClient::init(const ::std::string& zookeeperConnectString) {
    //validate the connection string
    if (zookeeperConnectString.find(PATH_DELIM) != ::std::string::npos) {
        THROW_EXCEPTION(ServiceDiscoveryException,
                "ZK connect string should not contain \"" +
                PATH_DELIM + "\": " + zookeeperConnectString);
    }

    //initialize our namespace
    initializeNamespace(zookeeperConnectString);

    //append our namespace for connection to zookeeper - this will serve as the chroot
    ::std::string namespacedConnectString = zookeeperConnectString +
            PATH_DELIM + NAMESPACE;

    //initialize a zookeeper connection without Watches
    if (ReturnCode::Ok != _handle.init(namespacedConnectString, DEFAULT_SESSION_TIMEOUT,
                                       ::boost::shared_ptr<Watch>())) {
        THROW_EXCEPTION(ServiceDiscoveryException, "Unable to connect to zookeeper");
    }
}


void ServiceDiscoveryClient::initializeNamespace(const ::std::string& connectString) {
    //initialize a zookeeper connection without a namespace CHROOT
    if (ReturnCode::Ok != _handle.init(connectString, DEFAULT_SESSION_TIMEOUT,
                                       ::boost::shared_ptr<Watch>())) {
        THROW_EXCEPTION(ServiceDiscoveryException, "Unable to connect to zookeeper");
    }

    /*
     * Since we're using a namespace, we have to ensure it's created.
     * No problem if its already created
     */
    ::std::string pathCreated;
    ReturnCode::type response = _handle.create(PATH_DELIM + NAMESPACE, "",
            SD_DEFAULT_ACL, CreateMode::Persistent, pathCreated);
    if (response != ReturnCode::Ok && response != ReturnCode::NodeExists) {
        ::std::ostringstream ss;
        ss << "Error in initializing namespace. ZK error: " << response;
        THROW_EXCEPTION(ServiceDiscoveryException, ss.str());
    }

    //close connection so we can reconnect with namespaced CHROOT
    _handle.close();
}


::std::vector< ::std::string> ServiceDiscoveryClient::splitPath(const ::std::string &absPath,
        const char *delimiter, bool keepEmpty)
{
    ::std::vector< ::std::string> paths;
    size_t prev = 0;
    size_t next = 0;

    while ((next = absPath.find_first_of(delimiter, prev)) != ::std::string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            paths.push_back(absPath.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < absPath.size())
    {
        paths.push_back(absPath.substr(prev));
    }

    return paths;
}


void ServiceDiscoveryClient::validateHostAndPort(const ::std::string& hostAndPort) {
    //TODO (ope): implement stronger validation. Using HostAndPort from EzConfiguration(C++)?
    if (hostAndPort.find(":") == ::std::string::npos) {
        THROW_EXCEPTION(ServiceDiscoveryException, "Invalid Host:Port format: " + hostAndPort);
    }
}

}} //namspace ::ezbake::ezdiscovery
