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
 * ServiceDiscoveryExceptions.h
 *
 *  Created on: Feb 28, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SERVICEDISCOVERYEXCEPTIONS_H_
#define EZBAKE_EZDISCOVERY_SERVICEDISCOVERYEXCEPTIONS_H_

#include <ezbake/ezdiscovery/ZKContrib.h>
#include <sstream>


#define THROW_EXCEPTION(T, X) throw T(X, __FUNCTION__, __FILE__)


namespace ezbake { namespace ezdiscovery {

/**
 * Wrapper class for hadoop exceptions
 */
class ServiceDiscoveryException : public ::hadoop::Exception {
public:
    static const ::std::string NAME;

public:
    ServiceDiscoveryException(const ::std::string& message,
                              const ::std::string& component="",
                              const ::std::string& location="",
                              const Exception* reason = NULL);

    virtual ServiceDiscoveryException* clone() const;

    virtual const char* getTypename() const;
};

}} // namespace ::ezbake::ezdiscovery

#endif /* EZBAKE_EZDISCOVERY_SERVICEDISCOVERYEXCEPTIONS_H_ */
