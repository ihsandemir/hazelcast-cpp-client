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
 * ErrorCodec.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: ihsan
 */

#include <sstream>
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                ErrorCodec ErrorCodec::decode(ClientMessage &message) {
                    return ErrorCodec(message);
                }

                ErrorCodec::ErrorCodec(ClientMessage &message) :
                    errorCode(message.getInt32()),
                    className(message.getStringUtf8()),
                    message(message.getStringUtf8()),
                    stackTrace(message.getArray<StackTraceElement>()),
                    causeErrorCode(message.getInt32()),
                    causeClassName(message.getNullable<std::string>()) {
                    assert(ErrorCodec::TYPE == message.getMessageType());
                }

                std::string ErrorCodec::toString() const {
                    std::ostringstream out;
                    out << "Error code:" << errorCode << ", Class name that generated the error:" << className << ", " << message << " Stack trace:" << std::endl ;
                    for (std::vector<StackTraceElement>::const_iterator it = stackTrace.begin(); it != stackTrace.end(); ++it) {
                        out << (*it) << std::endl;
                    }

                    out << std::endl << "Cause error code:" << causeErrorCode << std::endl;
                    if (NULL != causeClassName.get()) {
                        out << "Caused by:" << causeClassName << std::endl;
                    }
                    return out.str();
                }
            }
        }
    }
}
