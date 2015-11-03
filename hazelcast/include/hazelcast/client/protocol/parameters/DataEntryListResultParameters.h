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
 * DataEntryListResultParameters.h
 *
 *  Created on: Apr 21, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_DATAENTRYLISTRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_DATAENTRYLISTRESULTPARAMETERS_H_

#include "hazelcast/util/HazelcastDll.h"

#include <vector>
#include <memory>
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/protocol/ProtocolTypeDefs.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class DataEntryListResultParameters {
                public:
                    std::auto_ptr<DataArray> keys;
                    std::auto_ptr<DataArray> values;

                    static const protocol::ClientMessageType TYPE = protocol::DATA_LIST_RESULT;

                    static std::auto_ptr<DataEntryListResultParameters> decode(ClientMessage &message);

                private:
                    DataEntryListResultParameters();

                    DataEntryListResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_DATAENTRYLISTRESULTPARAMETERS_H_ */
