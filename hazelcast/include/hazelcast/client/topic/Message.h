/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include "hazelcast/client/TypedData.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class Member;

        namespace topic {
            class HAZELCAST_API Message {
            public:
                Message(const std::string &topicName, TypedData &&message, int64_t publishTime,
                        boost::optional<Member> &&member)
                        : messageObject(message), publishTime(publishTime), publishingMember(member), name(topicName) {}

                const TypedData &getMessageObject() const {
                    return messageObject;
                }

                int64_t getPublishTime() const {
                    return publishTime;
                }

                const Member *getPublishingMember() const {
                    return publishingMember.operator->();
                }

                const std::string &getSource() const {
                    return name;
                }

                const std::string &getName() const {
                    return name;
                }

            private:
                TypedData messageObject;
                int64_t publishTime;
                boost::optional<Member> publishingMember;
                std::string name;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

