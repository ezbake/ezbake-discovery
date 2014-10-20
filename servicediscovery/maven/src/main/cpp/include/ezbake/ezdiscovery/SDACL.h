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
 * SDACL.h
 *
 *  Created on: Feb 28, 2014
 *      Author: oarowojolu
 */

#ifndef EZBAKE_EZDISCOVERY_SDALC_H_
#define EZBAKE_EZDISCOVERY_SDALC_H_

#include <ezbake/ezdiscovery/ZKContrib.h>

namespace ezbake { namespace ezdiscovery {

/**
 * Service Discovery Access Control List
 *
 * Wrapper class for zookeeper access control lists
 */
class SDACL : public ::org::apache::zookeeper::data::ACL {
public:
    SDACL(const ::std::string& scheme, const ::std::string& id,
            ::org::apache::zookeeper::Permission::type permission) {
        getid().getscheme() = scheme;
        getid().getid() = id;
        setperms(permission);
    };

    SDACL(){};
    virtual ~SDACL(){};
};

}} // namespace ::ezbake::ezdiscovery

#endif /* EZBAKE_EZDISCOVERY_SDALC_H_ */
