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
 * GetPartitionsResultParameters.h
 *
 *  Created on: Apr 25, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_GETPARTITIONSRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_GETPARTITIONSRESULTPARAMETERS_H_

#include <memory>
#include "hazelcast/client/protocol/ProtocolTypeDefs.h"
#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class GetPartitionsResultParameters {
                public:
                    std::auto_ptr<AddressArray> members;
                    std::auto_ptr<ManagedInt32Array> ownerIndexes;

                    static const protocol::ClientMessageType TYPE = protocol::GET_PARTITIONS_RESULT;

                    static std::auto_ptr<GetPartitionsResultParameters> decode(ClientMessage &message);

                private:
                    GetPartitionsResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_GETPARTITIONSRESULTPARAMETERS_H_ */
