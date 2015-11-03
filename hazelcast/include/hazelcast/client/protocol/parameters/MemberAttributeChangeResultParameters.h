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
 * MemberAttributeChangeResultParameters.h
 *
 *  Created on: Apr 24, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_MEMBERATTRIBUTECHANGERESULTTPARAMETERS_H_
#define HAZELCAST_CLIENT_MEMBERATTRIBUTECHANGERESULTTPARAMETERS_H_

#include <memory>

#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class MemberAttributeChangeResultParameters {
                public:
                    std::auto_ptr<impl::MemberAttributeChange> attributeChange;

                    static const protocol::ClientMessageType TYPE = protocol::MEMBER_ATTRIBUTE_RESULT;

                    static std::auto_ptr<MemberAttributeChangeResultParameters> decode(ClientMessage &message);

                private:
                    MemberAttributeChangeResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_MEMBERATTRIBUTECHANGERESULTTPARAMETERS_H_ */
