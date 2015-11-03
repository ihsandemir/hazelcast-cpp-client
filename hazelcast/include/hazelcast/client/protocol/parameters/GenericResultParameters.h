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
 * GenericResultParameters.h
 *
 *  Created on: Apr 18, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_GENERICRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_GENERICRESULTPARAMETERS_H_

#include "hazelcast/util/HazelcastDll.h"
#include <stdint.h>
#include "hazelcast/client/protocol/ClientMessageType.h"
#include <memory>

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
                class GenericResultParameters {
                public:
                    std::auto_ptr<hazelcast::client::serialization::pimpl::Data> data;

                    static const protocol::ClientMessageType TYPE = protocol::RESULT;

                    static std::auto_ptr<GenericResultParameters> decode(ClientMessage &message);

                private:
                    GenericResultParameters();

                    GenericResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_GENERICRESULTPARAMETERS_H_ */
