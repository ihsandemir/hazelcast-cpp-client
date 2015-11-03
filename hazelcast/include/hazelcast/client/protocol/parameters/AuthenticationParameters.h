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
 * AuthenticationParameters.h
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_AUTHENTICATIONPARAMETERS_H_
#define HAZELCAST_CLIENT_AUTHENTICATIONPARAMETERS_H_

#include <string>
#include <memory>
#include <stdint.h>

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace parameters {
                class AuthenticationParameters {
                public:
                    std::auto_ptr<std::string> username;
                    std::auto_ptr<std::string> password;
                    std::auto_ptr<std::string> uuid;
                    std::auto_ptr<std::string> ownerUuid;
                    uint8_t isOwnerConnection;

                    AuthenticationParameters(const AuthenticationParameters &rhs);

                    virtual ~AuthenticationParameters();

                    /**
                    * Decode input byte array data into parameters
                    *
                    * @param flyweight
                    * @return AuthenticationParameters
                    */
                    static AuthenticationParameters decode(ClientMessage &flyweight);

                    /**
                    * Encode parameters into byte array, i.e. ClientMessage
                    *
                    * @param username
                    * @param password
                    * @param uuid
                    * @param ownerUuid
                    * @param isOwnerConnection
                    * @return encoded ClientMessage
                    */
                    static std::auto_ptr<ClientMessage> encode(const std::string & username,
                            const std::string & password, const std::string & uuid,
                            const std::string & ownerUuid,
                            uint8_t isOwnerConnection);

                    /**
                    * sample data size estimation
                    *
                    * @return size
                    */
                    static int32_t calculateDataSize(const std::string & username,
                            const std::string & password, const std::string & uuid,
                            const std::string & ownerUuid,
                            uint8_t isOwnerConnection);

                private:
                    AuthenticationParameters(ClientMessage &message);
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_AUTHENTICATIONPARAMETERS_H_ */
