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
//
// Created by ihsan demir on 10/11/15.

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_

#include <string>


#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        class Address;
        namespace protocol {
            class ClientMessage;
            namespace codec {
                class IAddListenerCodec;
            }
        }
        namespace spi {
            namespace impl {
                namespace listener {
                    HAZELCAST_API class EventRegistration {

                    public:
                        EventRegistration(int callId,
                                          const Address &member,
                                          std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec);

                        int32_t getCorrelationId() const;

                        const Address &getMemberAddress() const;

                        const protocol::codec::IAddListenerCodec *getAddCodec() const;

                        void setCorrelationId(int32_t callId);

                    private:
                        int32_t correlationId;
                        const Address &memberAddress;
                        std::auto_ptr<protocol::codec::IAddListenerCodec> addCodec;
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_EVENTREGISTRATION_H_
