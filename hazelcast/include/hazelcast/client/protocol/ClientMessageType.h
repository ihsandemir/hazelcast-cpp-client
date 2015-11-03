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
 * ClientMessageType.h
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_CLIENTMESSAGETYPE_H_
#define HAZELCAST_CLIENT_CLIENTMESSAGETYPE_H_

namespace hazelcast {
    namespace client {
        namespace protocol {

            enum ClientMessageType {
                /**
                * Default Authentication with user-name and password
                */
                AUTHENTICATION_DEFAULT_REQUEST = 1,

                /**
                * Custom Authentication with custom credentials impl
                */
                AUTHENTICATION_CUSTOM_REQUEST = 2,

                /**
                * Exception
                */
                EXCEPTION = 3,

                /**
                * Result wrapper message type
                */
                RESULT = 4,

                /**
                * Event registration id
                */
                ADD_LISTENER_RESULT = 5,

                ADD_ENTRY_LISTENER_EVENT = 6,

                ADD_ENTRY_LISTENER_REQUEST = 7,

                REGISTER_MEMBERSHIP_LISTENER_REQUEST = 8,

                REGISTER_MEMBERSHIP_LISTENER_EVENT = 9,

                MAP_PUT_REQUEST = 10,

                CREATE_PROXY_REQUEST = 11,

                GET_PARTITIONS_REQUEST = 12,

                GET_PARTITIONS_RESULT = 13,

                AUTHENTICATION_RESULT = 14,

                BOOLEAN_RESULT = 15,

                INTEGER_RESULT = 16,

                DATA_LIST_RESULT = 17,

                DATA_ENTRY_LIST_RESULT = 18,

                MEMBER_RESULT = 19,

                MEMBER_LIST_RESULT = 20,

                MEMBER_ATTRIBUTE_RESULT = 21,

                VOID_RESULT = 22,

                ENTRY_VIEW = 23,

                DESTROY_PROXY_REQUEST = 24,

                ITEM_EVENT = 25,

                TOPIC_EVENT = 26,

                LONG_RESULT = 27,

                PARTITION_LOST_EVENT = 28,

                REMOVE_ALL_LISTENERS = 29,

                ADD_PARTITION_LOST_LISTENER = 30,

                REMOVE_PARTITION_LOST_LISTENER = 31,

                GET_DISTRIBUTED_OBJECT = 32,

                ADD_DISTRIBUTED_OBJECT_LISTENER = 33,

                REMOVE_DISTRIBUTED_OBJECT_LISTENER = 34,

                TRANSACTION_RECOVER_ALL = 35,

                TRANSACTION_RECOVER = 36,

                TRANSACTION_CREATE_RESULT = 37,

                TRANSACTION_CREATE = 38,

                TRANSACTION_PREPARE = 39,

                TRANSACTION_COMMITMSG = 40, // Note that TRANSACTION_COMMIT and TRANSACTION_ROLLBACK are reserved words in winnt.h, hence had to use different names

                TRANSACTION_ROLLBACKMSG = 41,

                PING = 42,

                MAP_INT_DATA_RESULT = 43,

                MAP_DATA_DATA_RESULT = 44
            };
        }
    }
}

#endif /* HAZELCAST_CLIENT_CLIENTMESSAGETYPE_H_ */
