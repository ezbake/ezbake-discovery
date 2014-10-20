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
 * ServiceDiscoveryClient.h
 *
 *  Created on: Feb 28, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCLIENT_H_
#define EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCLIENT_H_

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <ezbake/ezdiscovery/SDACL.h>
#include <ezbake/ezdiscovery/ServiceDiscoveryExceptions.h>

namespace ezbake { namespace ezdiscovery {

class ServiceDiscoveryClient : private ::boost::noncopyable {
public:
    static const ::std::string NAMESPACE;

protected:
    static const unsigned int MAX_NUM_OF_TRIES = 5;
    static const unsigned int DEFAULT_SESSION_TIMEOUT = 30000; //ms
    static const ::std::string ENDPOINTS_ZK_PATH;
    static const ::std::string JUST_SERVICE_APP_NAME;
    static const ::std::string SECURITY_ZK_PATH;
    static const ::std::string SECURITY_ID_NODE;

    static const ::std::string PATH_DELIM;

    static const SDACL OPEN_ACL_UNSAFE_ACL;
    static const SDACL READ_ACL_UNSAFE_ACL;
    static const SDACL CREATOR_ALL_ACL_ACL;
    static const ::std::vector< ::org::apache::zookeeper::data::ACL> SD_DEFAULT_ACL;

public:
    /**
     * Constructor/Destructor
     */
    ServiceDiscoveryClient() {}
    virtual ~ServiceDiscoveryClient() {
        close();
    }

    /**
     * Terminates our connectio to zookeeper
     */
    void close();

    /**
     * Establishes the connection to zookeeper so we can look up services
     */
    void init(const ::std::string& zookeeperConnectString);

    /**
     * Seperates a given path into it's respective nodes
     */
    static ::std::vector< ::std::string> splitPath(const ::std::string &absPath,
            const char *delimiter = "/", bool keepEmpty = false);

    /**
     * Checks if the specified host and port is valid
     *
     * @throws ServiceDiscoveryException if host and port is not valid
     */
    static void validateHostAndPort(const ::std::string& hostAndPort);

    /*
     * API to create a zookeeper path from the passed strings.
     * This function is templatized - allowing for variable number of string arguments
     *
     * @param paths - variable number of string arguments
     *
     * @return - the generate path
     *
     * @throws ServiceDiscoveryException if path arguments are invalid
     */
    template<typename... Args>
    static const ::std::string makeZKPath(Args... paths) {
        ::std::string thepath = buildZKPath(paths...);

        if (thepath.find("//") == 0) {
            //strip any extra path delimiter at start of path
            thepath = thepath.substr(1);
        }

        return thepath;
    }

private:
    void initializeNamespace(const ::std::string& connectString);

    static const ::std::string buildZKPath() { return ""; }

    template<typename... Args>
    static const ::std::string buildZKPath(const ::std::string& path, Args... paths) {
        if (path.find(PATH_DELIM, 1) != ::std::string::npos) {
            THROW_EXCEPTION(ServiceDiscoveryException, "ZK path should not contain \"" + PATH_DELIM + "\": " + path);
        }
        return PATH_DELIM + path + buildZKPath(paths...);
    }

protected:
    org::apache::zookeeper::ZooKeeper _handle;
};

}} // namespace ::ezbake::ezdiscovery

#endif /* EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCLIENT_H_ */
