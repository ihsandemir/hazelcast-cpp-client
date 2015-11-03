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
 * EntryViewParameters.h
 *
 *  Created on: Apr 21, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_ENTRYVIEWPARAMETERS_H_
#define HAZELCAST_CLIENT_ENTRYVIEWPARAMETERS_H_

#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace map {
            class DataEntryView;
        }
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class EntryViewParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::ENTRY_VIEW;

                    std::auto_ptr<map::DataEntryView> entryView;

                    static std::auto_ptr<EntryViewParameters> decode(ClientMessage &message);

                    EntryViewParameters(const EntryViewParameters &rhs);
                private:
                    EntryViewParameters();

                    EntryViewParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_ENTRYVIEWPARAMETERS_H_ */
