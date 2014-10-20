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
 * ServiceDiscoveryCallbacks.h
 *
 *  Created on: Mar 1, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCALLBACKS_H_
#define EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCALLBACKS_H_

#include <ezbake/ezdiscovery/ZKContrib.h>

namespace ezbake {
namespace ezdiscovery {

/**
 * Service Discovery Callback
 *
 * Default wrapper class to ZooKeeper callback classes
 * This class is pseudo-abstract, meaning functionality depends on a sub-class
 * implementing the virtual process functions inorder to handle the response from the callback
 * invocation.
 */
class ServiceDiscoveryCallback : public ::org::apache::zookeeper::ExistsCallback,
                                 public ::org::apache::zookeeper::GetChildrenCallback,
                                 public ::org::apache::zookeeper::CreateCallback,
                                 public ::org::apache::zookeeper::RemoveCallback {
public:
    enum CallbackResponse {
        OK, //ZooKeeper command was successful
        ERROR //ZooKeeper command reported an error
    };

public:
    //Exists callback
    void process(::org::apache::zookeeper::ReturnCode::type rc,
            const ::std::string& path, const ::org::apache::zookeeper::data::Stat& stat) {
        if (rc != ::org::apache::zookeeper::ReturnCode::Ok &&
            rc != ::org::apache::zookeeper::ReturnCode::NoNode) {
            /*
             * ZooKeeper returned an error.
             * Send error to callback and indicate node was not found
             */
            process(ERROR, false);
        }

        /*
         * ZooKeeper did not report an error.
         * Send response via status to callback
         */
        process(OK, (rc == ::org::apache::zookeeper::ReturnCode::Ok));
    }

    //GetChildren callback
    void process(::org::apache::zookeeper::ReturnCode::type rc,
            const ::std::string& path, const ::std::vector< ::std::string>& children,
            const ::org::apache::zookeeper::data::Stat& stat) {
        if (rc != ::org::apache::zookeeper::ReturnCode::Ok &&
            rc != ::org::apache::zookeeper::ReturnCode::NoNode) {
            process(ERROR, children);
        }
        process(OK, children);
    }

    //Create callback
    virtual void process(::org::apache::zookeeper::ReturnCode::type rc,
            const ::std::string& pathRequested, const ::std::string& pathCreated) {
        if (rc != ::org::apache::zookeeper::ReturnCode::Ok &&
            rc != ::org::apache::zookeeper::ReturnCode::NodeExists) {
            process(ERROR);
        }
        process(OK);
    }

    //Remove callback
    void process(::org::apache::zookeeper::ReturnCode::type rc,
            const ::std::string& path) {
        if (rc != ::org::apache::zookeeper::ReturnCode::Ok &&
            rc != ::org::apache::zookeeper::ReturnCode::NoNode) {
            process(ERROR);
        }
        process(OK);
    }

private:

    /*
     * Empty process apis. Sub-class inheriting this class should expand handling
     * of the callback responses
     */
    virtual void process(CallbackResponse response) {}
    virtual void process(CallbackResponse response, bool status) {}
    virtual void process(CallbackResponse response, const ::std::string& value) {}
    virtual void process(CallbackResponse response, const ::std::vector< ::std::string>& values) {}
};


/*
 * Generic callback class for reporting only the operation response
 */
class ServiceDiscoveryOpCallback : public ServiceDiscoveryCallback {
public:
    virtual void process(CallbackResponse response) = 0;
};

/*
 * Callback class for reporting a boolean status:
 *  True - operation evaluated true
 *  False - operation evaluated false
 */
class ServiceDiscoveryStatusCallback : public ServiceDiscoveryCallback {
public:
    virtual void process(CallbackResponse response, bool status) = 0;
};

/*
 * Callback class for reporting a node from a list of children
 */
class ServiceDiscoveryNodeCallback : public ServiceDiscoveryCallback {
public:
    void process(CallbackResponse response, const ::std::vector< ::std::string>& values) {
        process(response, (values.size() ? values.at(0) : ""));
    }
    virtual void process(CallbackResponse response, const ::std::string& value) = 0;
};

/*
 * Callback class for reporting a list value
 */
class ServiceDiscoveryListCallback : public ServiceDiscoveryCallback {
public:
    virtual void process(CallbackResponse response, const ::std::vector< ::std::string>& values) = 0;
};

} //namespace ezdiscovery
} //namespace ezbake

#endif /* EZBAKE_EZDISCOVERY_SERVICEDISCOVERYCALLBACKS_H_ */
