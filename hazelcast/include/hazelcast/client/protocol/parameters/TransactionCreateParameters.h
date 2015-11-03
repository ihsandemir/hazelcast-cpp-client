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
 * TransactionCreateParameters.h
 *
 *  Created on: Apr 29, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_TRANSACTIONCREATEPARAMETERS_H
#define HAZELCAST_CLIENT_TRANSACTIONCREATEPARAMETERS_H

#include <memory>
#include <stdint.h>

#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class TransactionCreateParameters {
                public:
                    static const protocol::ClientMessageType TYPE = protocol::TRANSACTION_CREATE;

                    //SerializableXID xid;  // TODO: Use this type instead (the Java code has this class)
                    static std::auto_ptr<ClientMessage> encode(
                            bool xid, long timeout, int32_t durability, int32_t transactionType, long threadId);

                private:
                    static int32_t calculateDataSize(
                            bool xid, long timeout, int32_t durability, int32_t transactionType, long threadId);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_TRANSACTIONCREATEPARAMETERS_H
