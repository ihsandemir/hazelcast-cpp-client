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
 * TopicEventParameters.cpp
 *
 *  Created on: Apr 29, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/TopicEventParameters.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<TopicEventParameters> TopicEventParameters::decode(ClientMessage &message) {
                    return std::auto_ptr<TopicEventParameters>(new TopicEventParameters(message));
                }

                TopicEventParameters::TopicEventParameters(ClientMessage &message) :
                    message(message.getData()), publishTime(message.getInt64()), uuid(message.getStringUtf8()) {

                    assert(TYPE == message.getMessageType());
                }
            }
        }
    }
}
