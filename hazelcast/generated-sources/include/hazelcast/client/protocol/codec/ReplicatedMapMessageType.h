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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_REPLICATEDMAPMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_REPLICATEDMAPMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum ReplicatedMapMessageType {

                    REPLICATEDMAP_PUT=0x0e01,
                    REPLICATEDMAP_SIZE=0x0e02,
                    REPLICATEDMAP_ISEMPTY=0x0e03,
                    REPLICATEDMAP_CONTAINSKEY=0x0e04,
                    REPLICATEDMAP_CONTAINSVALUE=0x0e05,
                    REPLICATEDMAP_GET=0x0e06,
                    REPLICATEDMAP_REMOVE=0x0e07,
                    REPLICATEDMAP_PUTALL=0x0e08,
                    REPLICATEDMAP_CLEAR=0x0e09,
                    REPLICATEDMAP_ADDENTRYLISTENERTOKEYWITHPREDICATE=0x0e0a,
                    REPLICATEDMAP_ADDENTRYLISTENERWITHPREDICATE=0x0e0b,
                    REPLICATEDMAP_ADDENTRYLISTENERTOKEY=0x0e0c,
                    REPLICATEDMAP_ADDENTRYLISTENER=0x0e0d,
                    REPLICATEDMAP_REMOVEENTRYLISTENER=0x0e0e,
                    REPLICATEDMAP_KEYSET=0x0e0f,
                    REPLICATEDMAP_VALUES=0x0e10,
                    REPLICATEDMAP_ENTRYSET=0x0e11,
                    REPLICATEDMAP_ADDNEARCACHEENTRYLISTENER=0x0e12
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_REPLICATEDMAPMESSAGETYPE




