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
 * RegisterMembershipListenerParameters.h
 *
 *  Created on: Apr 16, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_REGISTERMEMBERSHIPLISTENEREVENTPARAMETERS_H_
#define HAZELCAST_CLIENT_REGISTERMEMBERSHIPLISTENEREVENTPARAMETERS_H_

#include <memory>
#include <stdint.h>
#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class RegisterMembershipListenerParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::REGISTER_MEMBERSHIP_LISTENER_REQUEST;

                    static std::auto_ptr<ClientMessage> encode();

                    /**
                    * sample data size estimation
                    *
                    * @return size
                    */
                    static int32_t calculateDataSize();

                private:
                    // Prevent public construction
                    RegisterMembershipListenerParameters(ClientMessage &message);
                    RegisterMembershipListenerParameters();
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_REGISTERMEMBERSHIPLISTENEREVENTPARAMETERS_H_ */
