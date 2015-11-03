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
 * GetPartitionsParameters.h
 *
 *  Created on: Apr 25, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_GETPARTITIONSPARAMETERS_H
#define HAZELCAST_CLIENT_GETPARTITIONSPARAMETERS_H

#include <memory>
#include <string>
#include <stdint.h>

#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class GetPartitionsParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::GET_PARTITIONS_REQUEST;

                    static std::auto_ptr<ClientMessage> encode();

                private:
                    static int32_t calculateDataSize();
                };
            }
        }
    }
}



#endif //HAZELCAST_CLIENT__DESTROYPROXYPARAMETERS_H
