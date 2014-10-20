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
 * ServiceDiscoveryExceptions.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: oarowojolu
 */

#include <ezbake/ezdiscovery/ServiceDiscoveryExceptions.h>
#include <boost/shared_ptr.hpp>

namespace ezbake { namespace ezdiscovery {

const std::string ServiceDiscoveryException::NAME = "ServiceDiscoveryException";

ServiceDiscoveryException::ServiceDiscoveryException(const ::std::string& message,
        const ::std::string& component, const ::std::string& location, const Exception* reason)
    : ::hadoop::Exception(message, component, location, reason)
{}

/*
 * Create a clone of this exception
 *
 * @return - the clone
 */
ServiceDiscoveryException* ServiceDiscoveryException::clone() const {
    boost::shared_ptr<ServiceDiscoveryException> p(new ServiceDiscoveryException(*this));
    return p.get();
}

/*
 * Get the name of this exception
 *
 * @return the name of this exception as char*
 */
const char* ServiceDiscoveryException::getTypename() const {
    return NAME.c_str();
}

}} // namespace ::ezbake::ezdiscovery
