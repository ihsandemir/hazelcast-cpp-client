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
 * EntryViewParameters.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/EntryViewParameters.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/map/DataEntryView.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<EntryViewParameters> EntryViewParameters::decode(ClientMessage &message) {
                    return std::auto_ptr<EntryViewParameters>(new EntryViewParameters(message));
                }

                EntryViewParameters::EntryViewParameters(ClientMessage &message) {

                    assert(TYPE == message.getMessageType());

                    bool isNull = message.getBoolean();
                    if (!isNull) {
                        entryView = std::auto_ptr<map::DataEntryView>(new map::DataEntryView());
                        entryView->key = *message.getData();
                        entryView->value = *message.getData();
                        entryView->cost = message.getUint64();
                        entryView->creationTime = message.getUint64();
                        entryView->expirationTime = message.getUint64();
                        entryView->hits = message.getUint64();
                        entryView->lastAccessTime = message.getUint64();
                        entryView->lastStoredTime = message.getUint64();
                        entryView->version = message.getUint64();
                        entryView->evictionCriteriaNumber = message.getUint64();
                        entryView->ttl = message.getUint64();
                    }

                }
            }
        }
    }
}
