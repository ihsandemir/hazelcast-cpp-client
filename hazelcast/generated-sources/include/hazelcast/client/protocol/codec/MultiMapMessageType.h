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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum MultiMapMessageType {

                    MULTIMAP_PUT=0x0201,
                    MULTIMAP_GET=0x0202,
                    MULTIMAP_REMOVE=0x0203,
                    MULTIMAP_KEYSET=0x0204,
                    MULTIMAP_VALUES=0x0205,
                    MULTIMAP_ENTRYSET=0x0206,
                    MULTIMAP_CONTAINSKEY=0x0207,
                    MULTIMAP_CONTAINSVALUE=0x0208,
                    MULTIMAP_CONTAINSENTRY=0x0209,
                    MULTIMAP_SIZE=0x020a,
                    MULTIMAP_CLEAR=0x020b,
                    MULTIMAP_VALUECOUNT=0x020c,
                    MULTIMAP_ADDENTRYLISTENERTOKEY=0x020d,
                    MULTIMAP_ADDENTRYLISTENER=0x020e,
                    MULTIMAP_REMOVEENTRYLISTENER=0x020f,
                    MULTIMAP_LOCK=0x0210,
                    MULTIMAP_TRYLOCK=0x0211,
                    MULTIMAP_ISLOCKED=0x0212,
                    MULTIMAP_UNLOCK=0x0213,
                    MULTIMAP_FORCEUNLOCK=0x0214,
                    MULTIMAP_REMOVEENTRY=0x0215
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_MULTIMAPMESSAGETYPE




