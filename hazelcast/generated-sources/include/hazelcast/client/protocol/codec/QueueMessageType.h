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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                enum QueueMessageType {

                    QUEUE_OFFER=0x0301,
                    QUEUE_PUT=0x0302,
                    QUEUE_SIZE=0x0303,
                    QUEUE_REMOVE=0x0304,
                    QUEUE_POLL=0x0305,
                    QUEUE_TAKE=0x0306,
                    QUEUE_PEEK=0x0307,
                    QUEUE_ITERATOR=0x0308,
                    QUEUE_DRAINTO=0x0309,
                    QUEUE_DRAINTOMAXSIZE=0x030a,
                    QUEUE_CONTAINS=0x030b,
                    QUEUE_CONTAINSALL=0x030c,
                    QUEUE_COMPAREANDREMOVEALL=0x030d,
                    QUEUE_COMPAREANDRETAINALL=0x030e,
                    QUEUE_CLEAR=0x030f,
                    QUEUE_ADDALL=0x0310,
                    QUEUE_ADDLISTENER=0x0311,
                    QUEUE_REMOVELISTENER=0x0312,
                    QUEUE_REMAININGCAPACITY=0x0313,
                    QUEUE_ISEMPTY=0x0314
                };
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_PROTOCOL_CODEC_QUEUEMESSAGETYPE




