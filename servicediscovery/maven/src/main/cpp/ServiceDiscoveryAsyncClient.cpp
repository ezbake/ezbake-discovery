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

 * ServiceDiscoveryAsyncClient.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: oarowojolu
 */

#include <ezbake/ezdiscovery/ServiceDiscoveryAsyncClient.h>
#include <boost/make_shared.hpp>


namespace ezbake { namespace ezdiscovery {

using namespace org::apache::zookeeper;


void ServiceDiscoveryAsyncClient::registerEndpoint(const ::std::string& serviceName,
        const ::std::string& point, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    registerEndpoint(JUST_SERVICE_APP_NAME, serviceName, point, callback);
}


void ServiceDiscoveryAsyncClient::registerEndpoint(const ::std::string& appName,
        const ::std::string& serviceName, const ::std::string& point,
        ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    validateHostAndPort(point); //validate the host and port for the point

    createPath(makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH, point), callback);
}


void ServiceDiscoveryAsyncClient::unregisterEndpoint(const ::std::string& serviceName,
        const ::std::string& point, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    unregisterEndpoint(JUST_SERVICE_APP_NAME, serviceName, point, callback);
}


void ServiceDiscoveryAsyncClient::unregisterEndpoint(const ::std::string& appName,
        const ::std::string& serviceName, const ::std::string& point,
        ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    ::std::string path = makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH, point);

    //delete the endpoint
    if (ReturnCode::Ok != _handle.remove(path, -1, callback)) {
        THROW_EXCEPTION(ServiceDiscoveryException,
                "Error in unregistering endpoint. ZK error: error in dispatching request");
    }
}


void ServiceDiscoveryAsyncClient::getApplications(::boost::shared_ptr<ServiceDiscoveryListCallback> callback) {
    return getChildren("", callback);
}


void ServiceDiscoveryAsyncClient::getServices(::boost::shared_ptr<ServiceDiscoveryListCallback> callback) {
    return getServices(JUST_SERVICE_APP_NAME, callback);
}


void ServiceDiscoveryAsyncClient::getServices(const ::std::string& appName,
        ::boost::shared_ptr<ServiceDiscoveryListCallback> callback) {
    return getChildren(makeZKPath(appName), callback);
}


void ServiceDiscoveryAsyncClient::getEndpoints(const ::std::string& serviceName,
            ::boost::shared_ptr<ServiceDiscoveryListCallback> callback) {
    return getEndpoints(JUST_SERVICE_APP_NAME, serviceName, callback);
}


void ServiceDiscoveryAsyncClient::getEndpoints(const ::std::string& appName, const ::std::string& serviceName,
        ::boost::shared_ptr<ServiceDiscoveryListCallback> callback) {
    return getChildren(makeZKPath(appName, serviceName, ENDPOINTS_ZK_PATH), callback);
}


void ServiceDiscoveryAsyncClient::setSecurityIdForApplication(const ::std::string& applicationName,
        const ::std::string& securityId, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    createPath(makeZKPath(applicationName,
            SECURITY_ZK_PATH, SECURITY_ID_NODE, securityId), callback);
}


void ServiceDiscoveryAsyncClient::setSecurityIdForCommonService(const ::std::string& serviceName,
        const ::std::string& securityId, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback) {
    createPath(makeZKPath(JUST_SERVICE_APP_NAME,
            serviceName, SECURITY_ZK_PATH, SECURITY_ID_NODE, securityId), callback);
}


void ServiceDiscoveryAsyncClient::getSecurityIdForApplication(const ::std::string& applicationName,
        ::boost::shared_ptr<ServiceDiscoveryNodeCallback> callback) {
    getChildren(makeZKPath(applicationName, SECURITY_ZK_PATH, SECURITY_ID_NODE), callback);
}


void ServiceDiscoveryAsyncClient::getSecurityIdForCommonService(const ::std::string& serviceName,
        ::boost::shared_ptr<ServiceDiscoveryNodeCallback> callback) {
    getChildren(makeZKPath(JUST_SERVICE_APP_NAME, serviceName, SECURITY_ZK_PATH, SECURITY_ID_NODE),
            callback);
}


void ServiceDiscoveryAsyncClient::isServiceCommon(const ::std::string& serviceName,
        ::boost::shared_ptr<ServiceDiscoveryStatusCallback> callback) {
    return checkPathExists(makeZKPath(JUST_SERVICE_APP_NAME, serviceName), callback);
}

void ServiceDiscoveryAsyncClient::checkPathExists(const ::std::string& path,
        ::boost::shared_ptr<ServiceDiscoveryCallback> callback) {

    if (ReturnCode::Ok != _handle.exists(path,
                                         ::boost::shared_ptr<Watch>(),
                                         callback)) {
        THROW_EXCEPTION(ServiceDiscoveryException,
                "Error in checking path existence. ZK error: error in dispatching request");
    }
}

void ServiceDiscoveryAsyncClient::createPath(const std::string& path,
        ::boost::shared_ptr<ServiceDiscoveryCallback> callback) {

    //Don't handle callback from remove. If the node doesn't exists, we are going to create it
    _handle.remove(path, -1, ::boost::shared_ptr<RemoveCallback>());

    //pass creation of nodes along path to our delegate
    ::boost::shared_ptr<CreatePathDelegate> delegate =
            ::boost::make_shared<CreatePathDelegate>(*this, path,
                    ::boost::dynamic_pointer_cast<ServiceDiscoveryOpCallback>(callback));

    //use a shared_ptr to invoke the createPath method as the createPath method uses the
    //shared_ptr to hold a reference to the delegate object event after this current function
    //returns.
    delegate->createPath();
}


void ServiceDiscoveryAsyncClient::getChildren(const std::string& path,
        ::boost::shared_ptr<ServiceDiscoveryCallback> callback) {
    if (ReturnCode::Ok != _handle.getChildren(path,
                                              ::boost::shared_ptr<Watch>(),
                                              callback)) {
        THROW_EXCEPTION(ServiceDiscoveryException,
                "Error in getting nodes. ZK error: error in dispatching request");
    }
}


void ServiceDiscoveryAsyncClient::CreatePathDelegate::createPath() {
    //check for invalid node along path
    std::size_t pos = _principalPath.find("//", 1);
    if ((pos != std::string::npos) && (pos >=1)) {
        THROW_EXCEPTION(ServiceDiscoveryException,
                "Invalid path: " + _principalPath);
    }

    std::vector<std::string> pathNodes = splitPath(_principalPath);

    if (0 == pathNodes.size()) {
        //invalid path. Report error to principal callback
        _principalCallback->process(ServiceDiscoveryCallback::ERROR);
    } else {
        //create first node along path
        createPath(PATH_DELIM + pathNodes[0]);
    }
}


void ServiceDiscoveryAsyncClient::CreatePathDelegate::createPath(const std::string& path) {
    /*
     * Initiate the creation of a particular absolute path.
     * All parent nodes of path must exist for success.
     */

    if (ReturnCode::Ok != _client._handle.create(path, "",
            SD_DEFAULT_ACL, CreateMode::Persistent, shared_from_this())) {
        /*
         * Error in dispatching call to create, inform our principal callback of error and abort
         * creating the path
         */
        _principalCallback->process(ServiceDiscoveryCallback::ERROR);
    }
}


void  ServiceDiscoveryAsyncClient::CreatePathDelegate::process(ReturnCode::type rc,
        const std::string& pathRequested, const std::string& pathCreated) {

    if (rc != org::apache::zookeeper::ReturnCode::Ok &&
        rc != org::apache::zookeeper::ReturnCode::NodeExists) {
        //error reported by ZooKeeper. Forward to our principal callback
        _principalCallback->process(ServiceDiscoveryCallback::ERROR);
    }
    else if (pathCreated == _principalPath) {
        //this is last node along path. Full path was created. Return result to principal callback.
        _principalCallback->process(ServiceDiscoveryCallback::OK);
    }
    else if ((rc == org::apache::zookeeper::ReturnCode::Ok) && (pathCreated != pathRequested)){
        //Since we did not request for a sequential node, the path requested must match that created
        _principalCallback->process(ServiceDiscoveryCallback::ERROR);
    }
    else if ((rc == org::apache::zookeeper::ReturnCode::Ok) && (_principalPath.find(pathCreated) != 0)) {
        //path created is not a prefix of the requested principal path
        _principalCallback->process(ServiceDiscoveryCallback::ERROR);
    }
    else {
        /*
         * We still have more nodes along the path to create.
         * Create them. This delegate instance will handle the callback responses from ZooKeeper
         */
        size_t numNodesCreated = splitPath(pathRequested).size();
        std::string nextNode = splitPath(_principalPath).at(numNodesCreated);
        std::string pathToCreate = pathRequested + PATH_DELIM + nextNode;
        createPath(pathToCreate);
    }
}

}} // namespace ::ezbake::ezdiscovery
