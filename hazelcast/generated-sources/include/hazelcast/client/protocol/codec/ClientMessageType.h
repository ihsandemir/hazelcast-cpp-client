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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum ClientMessageType {

                    CLIENT_AUTHENTICATION=0x2,
                    CLIENT_AUTHENTICATIONCUSTOM=0x3,
                    CLIENT_ADDMEMBERSHIPLISTENER=0x4,
                    CLIENT_CREATEPROXY=0x5,
                    CLIENT_DESTROYPROXY=0x6,
                    CLIENT_GETPARTITIONS=0x8,
                    CLIENT_REMOVEALLLISTENERS=0x9,
                    CLIENT_ADDPARTITIONLOSTLISTENER=0xa,
                    CLIENT_REMOVEPARTITIONLOSTLISTENER=0xb,
                    CLIENT_GETDISTRIBUTEDOBJECTS=0xc,
                    CLIENT_ADDDISTRIBUTEDOBJECTLISTENER=0xd,
                    CLIENT_REMOVEDISTRIBUTEDOBJECTLISTENER=0xe,
                    CLIENT_PING=0xf
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTMESSAGETYPE




