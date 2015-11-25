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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum AtomicLongMessageType {

                    ATOMICLONG_APPLY=0x0a01,
                    ATOMICLONG_ALTER=0x0a02,
                    ATOMICLONG_ALTERANDGET=0x0a03,
                    ATOMICLONG_GETANDALTER=0x0a04,
                    ATOMICLONG_ADDANDGET=0x0a05,
                    ATOMICLONG_COMPAREANDSET=0x0a06,
                    ATOMICLONG_DECREMENTANDGET=0x0a07,
                    ATOMICLONG_GET=0x0a08,
                    ATOMICLONG_GETANDADD=0x0a09,
                    ATOMICLONG_GETANDSET=0x0a0a,
                    ATOMICLONG_INCREMENTANDGET=0x0a0b,
                    ATOMICLONG_GETANDINCREMENT=0x0a0c,
                    ATOMICLONG_SET=0x0a0d
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_ATOMICLONGMESSAGETYPE




