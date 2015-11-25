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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum SetMessageType {

                    SET_SIZE=0x0601,
                    SET_CONTAINS=0x0602,
                    SET_CONTAINSALL=0x0603,
                    SET_ADD=0x0604,
                    SET_REMOVE=0x0605,
                    SET_ADDALL=0x0606,
                    SET_COMPAREANDREMOVEALL=0x0607,
                    SET_COMPAREANDRETAINALL=0x0608,
                    SET_CLEAR=0x0609,
                    SET_GETALL=0x060a,
                    SET_ADDLISTENER=0x060b,
                    SET_REMOVELISTENER=0x060c,
                    SET_ISEMPTY=0x060d
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_SETMESSAGETYPE




