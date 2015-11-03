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
 * AddListenerResultParameters.h
 *
 *  Created on: Apr 21, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_ADDLISTENERRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_ADDLISTENERRESULTPARAMETERS_H_

#include <string>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class AddListenerResultParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::ADD_LISTENER_RESULT;

                    std::auto_ptr<std::string> registrationId;

                    static std::auto_ptr<AddListenerResultParameters> decode(ClientMessage &message);

                    AddListenerResultParameters(const AddListenerResultParameters &rhs);
                private:
                    AddListenerResultParameters();

                    AddListenerResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_ADDLISTENERRESULTPARAMETERS_H_ */
