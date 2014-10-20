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
 * ServiceDiscoverySyncClient.h
 *
 *  Created on: Feb 25, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SERVICEDISCOVERYSYNCCLIENT_H_
#define EZBAKE_EZDISCOVERY_SERVICEDISCOVERYSYNCCLIENT_H_

#include <ezbake/ezdiscovery/ServiceDiscoveryClient.h>

namespace ezbake { namespace ezdiscovery {

/**
 * Synchronous Service Discovery Client
 */
class ServiceDiscoverySyncClient : public ServiceDiscoveryClient {
public:
    /**
     * Constructor/Destructor
     */
    ServiceDiscoverySyncClient() {}
    virtual ~ServiceDiscoverySyncClient() {}

    /**
     * Register a service end point for service discovery
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *@param point the host:port number of a service end point
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void registerEndpoint(const ::std::string& serviceName, const ::std::string& point);
    void registerEndpoint(const ::std::string& appName, const ::std::string& serviceName, const ::std::string& point);

    /**
     * Unregister a service end point for service discovery
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *@param point the host:port number of a service end point
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void unregisterEndpoint(const ::std::string& serviceName, const ::std::string& point);
    void unregisterEndpoint(const ::std::string& appName, const ::std::string& serviceName, const ::std::string& point);

    /**
     * Get all applications registered with the client
     *
     * @return a list of strings containing the application names
     *
     * @throws ServiceDiscoveryException on any errors
     */
    ::std::vector< ::std::string> getApplications();
    ::std::vector< ::std::string> getServices();

    /**
     * Get all services registered under a given application
     *
     * @param appName the application for which to list services
     *
     * @return a list of strings corresponding to the service names for the application
     *
     * @throws ServiceDiscoveryException on zookeeper errors
     */
    ::std::vector< ::std::string> getServices(const ::std::string& appName);

    /**
     * Get the end points for a service in an application
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *
     *@return a list of strings containt host:port strings for the service end points
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    ::std::vector< ::std::string> getEndpoints(const ::std::string& serviceName);
    ::std::vector< ::std::string> getEndpoints(const ::std::string& appName, const ::std::string& serviceName);

    /**
     * Sets the security Id for an application
     *
     *@param applicationName the name of the application that we are setting it's security Id
     *@param securityId the security Id we are setting
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void setSecurityIdForApplication(const ::std::string& applicationName, const ::std::string& securityId);

    /**
     * Sets the security Id for a common service
     *
     *@param serviceName the name of the common service that we are setting it's security Id
     *@param securityId the security Id we are setting
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void setSecurityIdForCommonService(const ::std::string& serviceName, const ::std::string& securityId);

    /**
     * Gets the security Id for an application
     *
     *@param applicationName the name of the application that we are getting it's security Id
     *
     *@return the security Id of the application
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    const ::std::string getSecurityIdForApplication(const ::std::string& applicationName);

    /**
     * Gets the security Id for a common service
     *
     *@param serviceName the name of the common service that we are getting it's security Id
     *
     *@return the security Id of the common service
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    const ::std::string getSecurityIdForCommonService(const ::std::string& serviceName);

    /**
     * Determine if a service is common or not
     *
     *@param serviceName the name of the service
     *
     *@return boolean
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    bool isServiceCommon(const ::std::string& serviceName);

protected:
    virtual bool checkPathExists(const ::std::string& path);
    virtual void createPath(const ::std::string& path);
    virtual ::std::vector< ::std::string> getChildren(const ::std::string& path);
};

}} //namspace ::ezbake::ezdiscovery

#endif /* EZBAKE_EZDISCOVERY_SERVICEDISCOVERYSYNCCLIENT_H_ */
