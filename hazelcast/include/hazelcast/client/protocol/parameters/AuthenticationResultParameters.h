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
 * AuthenticationResultParameters.h
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_AUTHENTICATIONRESULTPARAMETERS_H_
#define HAZELCAST_CLIENT_AUTHENTICATIONRESULTPARAMETERS_H_

#include <string>
#include <memory>

#include "hazelcast/client/protocol/ClientMessageType.h"

namespace hazelcast {
    namespace client {
        class Address;

        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class AuthenticationResultParameters {
                public:
                    std::auto_ptr<Address> address;
                    std::auto_ptr<std::string> uuid;
                    std::auto_ptr<std::string> ownerUuid;

                    static const protocol::ClientMessageType TYPE = protocol::AUTHENTICATION_RESULT;

                    /**
                    * Decode input byte array data into parameters
                    *
                    * @param flyweight
                    * @return AuthenticationResultParameters
                    */
                    static std::auto_ptr<AuthenticationResultParameters> decode(ClientMessage &message);

                private:
                    AuthenticationResultParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_AUTHENTICATIONRESULTPARAMETERS_H_ */
