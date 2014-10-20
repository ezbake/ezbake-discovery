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
 * ServiceDiscoveryAsyncClient.h
 *
 *  Created on: Feb 25, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SERVICEDISCOVERYASYNCCLIENT_H_
#define EZBAKE_EZDISCOVERY_SERVICEDISCOVERYASYNCCLIENT_H_

#include <ezbake/ezdiscovery/ServiceDiscoveryClient.h>
#include <ezbake/ezdiscovery/ServiceDiscoveryCallbacks.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


namespace ezbake { namespace ezdiscovery {

/**
 * Asynchronous Service Discovery Client
 */
class ServiceDiscoveryAsyncClient : public ServiceDiscoveryClient {
public:
    /**
     * Constructor/Destructor
     */
    ServiceDiscoveryAsyncClient() {}
    virtual ~ServiceDiscoveryAsyncClient() {}

    /**
     * Register a service end point for service discovery
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *@param point the host:port number of a service end point
     *@param callback create callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void registerEndpoint(const ::std::string& serviceName, const ::std::string& point,
            ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);
    void registerEndpoint(const ::std::string& appName, const ::std::string& serviceName,
            const ::std::string& point, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);

    /**
     * Unregister a service end point for service discovery
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *@param point the host:port number of a service end point
     *@param callback remove callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void unregisterEndpoint(const ::std::string& serviceName, const ::std::string& point,
            ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);
    void unregisterEndpoint(const ::std::string& appName, const ::std::string& serviceName,
            const ::std::string& point, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);

    /**
     * Get all applications registered with the client
     *
     * @param callback remove callback that will be called for asynchronous response
     *
     * @throws ServiceDiscoveryException on any errors
     */
    void getApplications(::boost::shared_ptr<ServiceDiscoveryListCallback> callback);

    /**
     * Get all services registered under a given application
     *
     * @param appName the application for which to list services
      *@param callback get children callback that will be called for asynchronous response
     *
     * @throws ServiceDiscoveryException on zookeeper errors
     */
    void getServices(::boost::shared_ptr<ServiceDiscoveryListCallback> callback);
    void getServices(const ::std::string& appName,
            ::boost::shared_ptr<ServiceDiscoveryListCallback> callback);

    /**
     * Get the end points for a service in an application
     *
     *@param appName the name of the application that we are registering the service for
     *@param serviceName the name of the service that we are registering
     *@param callback get children callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void getEndpoints(const ::std::string& serviceName,
            ::boost::shared_ptr<ServiceDiscoveryListCallback> callback);
    void getEndpoints(const ::std::string& appName, const ::std::string& serviceName,
            ::boost::shared_ptr<ServiceDiscoveryListCallback> callback);

    /**
     * Sets the security Id for an application
     *
     *@param applicationName the name of the application that we are setting it's security Id
     *@param securityId the security Id we are setting
     *@param callback create callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void setSecurityIdForApplication(const ::std::string& applicationName, const ::std::string& securityId,
            ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);

    /**
     * Sets the security Id for a common service
     *
     *@param serviceName the name of the common service that we are setting it's security Id
     *@param securityId the security Id we are setting
     *@param callback create callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void setSecurityIdForCommonService(const ::std::string& serviceName, const ::std::string& securityId,
            ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback);

    /**
     * Gets the security Id for an application
     *
     *@param applicationName the name of the application that we are getting it's security Id
     *@param callback get children callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void getSecurityIdForApplication(const ::std::string& applicationName,
            ::boost::shared_ptr<ServiceDiscoveryNodeCallback> callback);

    /**
     * Gets the security Id for a common service
     *
     *@param serviceName the name of the common service that we are getting it's security Id
     *@param callback get children callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     */
    void getSecurityIdForCommonService(const ::std::string& serviceName,
            ::boost::shared_ptr<ServiceDiscoveryNodeCallback> callback);

    /**
     * Determine if a service is common or not
     *
     *@param serviceName the name of the service
     *@param callback exists callback that will be called for asynchronous response
     *
     *@throws ServiceDiscoveryException for any zookeeper errors
     *
     *@return boolean
     */
    void isServiceCommon(const ::std::string& serviceName,
            ::boost::shared_ptr<ServiceDiscoveryStatusCallback> callback);

protected:
    virtual void checkPathExists(const ::std::string& path,
            ::boost::shared_ptr<ServiceDiscoveryCallback> callback);

    virtual void createPath(const ::std::string& path,
            ::boost::shared_ptr<ServiceDiscoveryCallback> callback);

    virtual void getChildren(const ::std::string& path,
            ::boost::shared_ptr<ServiceDiscoveryCallback> callback);

private:

    /**
     * Delegate class that handles creating each node along a specified
     * path to create
     */
    class CreatePathDelegate : public ServiceDiscoveryCallback,
                               public ::boost::enable_shared_from_this<CreatePathDelegate> {
    public:
        /*
         * Our Create Path Delegate
         * This delegate class helps in ensuring all the nodes of a requested path to create
         * are created asynchronously. The class reports success at the creation of the last
         * node in the requested path or an error as soon as it occurs along the creation of
         * the path.
         * Success/Errors are reported via the callback provided at object instantiation.
         *
         *@param client reference to the asynchronous client requested in the path created
         *@param path the full path to be created
         *@param callback create callback that will be called for asynchronous response
         */
        CreatePathDelegate(ServiceDiscoveryAsyncClient& client,
                const ::std::string& path, ::boost::shared_ptr<ServiceDiscoveryOpCallback> callback)
                : _client(client),
                  _principalPath(path),
                  _principalCallback(callback) {}

        virtual ~CreatePathDelegate() {}

        /**
         * Starting point for our delegate path creation
         */
        void createPath();

        /**
         * Handle asynchronous callback from ZooKeeper for the creation of the requested path
         *
         *@param rc return code from ZooKeeper
         *@param pathRequested the path we requested to be created
         *@param pathCreated the path that was actually created
         *
         *@throws ServiceDiscoveryException for any zookeeper errors
         */
        virtual void process(org::apache::zookeeper::ReturnCode::type rc,
                const ::std::string& pathRequested, const ::std::string& pathCreated);

    private:
        void createPath(const ::std::string& path);

    private:
        ServiceDiscoveryAsyncClient& _client;
        ::std::string _principalPath;
        ::boost::shared_ptr<ServiceDiscoveryOpCallback> _principalCallback;
    };
    friend class CreatePathDelegate;
};

} /* namespace ezdiscovery */
} /* namespace ezbake */

#endif /* EZBAKE_EZDISCOVERY_SERVICEDISCOVERYASYNCCLIENT_H_ */
