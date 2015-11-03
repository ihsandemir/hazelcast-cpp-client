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
#ifndef HAZELCAST_CLIENT_MAPPUTALLPARAMETERS_H_
#define HAZELCAST_CLIENT_MAPPUTALLPARAMETERS_H_

#include <memory>
#include <stdint.h>
#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/GeneratedMessageType.h"
#include "hazelcast/client/protocol/ProtocolTypeDefs.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class HAZELCAST_API MapPutAllParameters {
                public:
                    static const protocol::GeneratedMessageType TYPE = protocol::MAP_PUTALL;

                    static std::auto_ptr<ClientMessage> encode(
                            const std::string &name, 
                            const protocol::DataArray &keys, 
                            const protocol::DataArray &values);

                    static int32_t calculateDataSize(
                            const std::string &name, 
                            const protocol::DataArray &keys, 
                            const protocol::DataArray &values);

                private:
                    // Preventing public access to constructors
                    MapPutAllParameters ();
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_MAPPUTALLPARAMETERS_H_ */
