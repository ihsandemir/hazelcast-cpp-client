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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum TransactionalMapMessageType {

                    TRANSACTIONALMAP_CONTAINSKEY=0x1001,
                    TRANSACTIONALMAP_GET=0x1002,
                    TRANSACTIONALMAP_GETFORUPDATE=0x1003,
                    TRANSACTIONALMAP_SIZE=0x1004,
                    TRANSACTIONALMAP_ISEMPTY=0x1005,
                    TRANSACTIONALMAP_PUT=0x1006,
                    TRANSACTIONALMAP_SET=0x1007,
                    TRANSACTIONALMAP_PUTIFABSENT=0x1008,
                    TRANSACTIONALMAP_REPLACE=0x1009,
                    TRANSACTIONALMAP_REPLACEIFSAME=0x100a,
                    TRANSACTIONALMAP_REMOVE=0x100b,
                    TRANSACTIONALMAP_DELETE=0x100c,
                    TRANSACTIONALMAP_REMOVEIFSAME=0x100d,
                    TRANSACTIONALMAP_KEYSET=0x100e,
                    TRANSACTIONALMAP_KEYSETWITHPREDICATE=0x100f,
                    TRANSACTIONALMAP_VALUES=0x1010,
                    TRANSACTIONALMAP_VALUESWITHPREDICATE=0x1011
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_TRANSACTIONALMAPMESSAGETYPE




