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
 * MembershipListResultParameters.h
 *
 *  Created on: Apr 16, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_MEMBERLISTRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_MEMBERLISTRESULTPARAMETERS_H_

#include <memory>
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class MembershipListResultParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::REGISTER_MEMBERSHIP_LISTENER_EVENT;

                    std::auto_ptr<std::vector<Member *> > memberList;

                    static MembershipListResultParameters decode(ClientMessage &message);

                    MembershipListResultParameters(ClientMessage &message);

                private:
                    // Prevent public construction
                    MembershipListResultParameters();
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_MEMBERLISTRESULTPARAMETERS_H_ */
