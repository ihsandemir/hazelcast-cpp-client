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
 * EntryEventParameters.cpp
 *
 *  Created on: Apr 29, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/EntryEventParameters.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<EntryEventParameters> EntryEventParameters::decode(ClientMessage &message) {
                    return std::auto_ptr<EntryEventParameters>(new EntryEventParameters(message));
                }

                EntryEventParameters::EntryEventParameters(ClientMessage &message) :
                    key(message.getData()), value(message.getData()), oldValue(message.getData()),
                    mergingValue(message.getData()), eventType(message.getInt32()), uuid(message.getStringUtf8()),
                    numberOfAffectedEntries(message.getInt32()) {

                    assert(TYPE == message.getMessageType());
                }
            }
        }
    }
}
