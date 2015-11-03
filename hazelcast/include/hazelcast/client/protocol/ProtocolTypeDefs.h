/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * ProtocolTypeDefs.h
 *
 *  Created on: May 14, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_PROTOCOLTYPEDEFS_H_
#define HAZELCAST_CLIENT_PROTOCOLTYPEDEFS_H_

#include <stdint.h>

#include "hazelcast/client/common/containers/ManagedPointerVector.h"
#include "hazelcast/client/common/containers/ManagedArray.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace protocol {
            /**
             * These typedefs shall enable us to switch to work with some other container types just
             * by changing these lines, with only minor modification of the code.
             */
            /**
             * We can not use stl container such as vector of auto_ptr, since auto_ptr does not conform the
             * stl container requirements: CopyConstructable, etc. That is why we are using a custom ManagedPointerVector.
             * This can be modified by changing this line when switched to C++11.
             */
            typedef common::containers::ManagedPointerVector<serialization::pimpl::Data> DataArray;
            typedef common::containers::ManagedPointerVector<Address> AddressArray;
            typedef common::containers::ManagedPointerVector<Member> MemberArray;
            typedef common::containers::ManagedArray<int32_t> ManagedInt32Array;
        }
    }
}

#endif /* HAZELCAST_CLIENT_PROTOCOLTYPEDEFS_H_ */
