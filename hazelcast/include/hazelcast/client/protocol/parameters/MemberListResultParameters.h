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
 * MemberListResultParameters.h
 *
 *  Created on: Apr 23, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_MEMBERLISTRESULTTPARAMETERS_H_
#define HAZELCAST_CLIENT_MEMBERLISTRESULTTPARAMETERS_H_

#include <vector>
#include <memory>

#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/common/containers/ManagedPointerVector.h"
#include "MemberResultParameters.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class MemberListResultParameters {
                public:
                    std::auto_ptr<common::containers::ManagedPointerVector<Member> > memberList;

                    static const protocol::ClientMessageType TYPE = protocol::MEMBER_LIST_RESULT;

                    static std::auto_ptr<MemberListResultParameters> decode(ClientMessage &message);

                private:
                    MemberListResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_MEMBERLISTRESULTTPARAMETERS_H_ */
