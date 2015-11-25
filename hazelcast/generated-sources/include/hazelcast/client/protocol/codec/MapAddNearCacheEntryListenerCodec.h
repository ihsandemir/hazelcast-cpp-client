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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPADDNEARCACHEENTRYLISTENERCODEC_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPADDNEARCACHEENTRYLISTENERCODEC_H_

#include <memory>


#include "hazelcast/client/protocol/codec/MapMessageType.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/codec/IAddListenerCodec.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            namespace codec {
                class HAZELCAST_API MapAddNearCacheEntryListenerCodec : public IAddListenerCodec{
                public:
                    //************************ REQUEST STARTS ******************************************************************//
                    class RequestParameters {
                        public:
                            static const enum MapMessageType TYPE;
                            static const bool RETRYABLE;

                        static std::auto_ptr<ClientMessage> encode(
                                const std::string &name, 
                                int32_t listenerFlags, 
                                bool localOnly);

                        static int32_t calculateDataSize(
                                const std::string &name, 
                                int32_t listenerFlags, 
                                bool localOnly);

                        private:
                            // Preventing public access to constructors
                            RequestParameters();
                    };
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class ResponseParameters {
                        public:
                            static const int TYPE;

                            std::string response;
                            
                            static ResponseParameters decode(ClientMessage &clientMessage);

                            // define copy constructor (needed for auto_ptr variables)
                            ResponseParameters(const ResponseParameters &rhs);
                        private:
                            ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                    //************************ EVENTS START*********************************************************************//
                    class AbstractEventHandler : public impl::BaseEventHandler {
                        public:
                        void handle(std::auto_ptr<protocol::ClientMessage> message);

                        virtual void handleIMapInvalidation(std::auto_ptr<serialization::pimpl::Data > key) = 0;

                        virtual void handleIMapBatchInvalidation(const std::vector<serialization::pimpl::Data > &keys) = 0;

                    };

                    //************************ EVENTS END **********************************************************************//

                    MapAddNearCacheEntryListenerCodec (const std::string &name, const int32_t &listenerFlags, const bool &localOnly);

                    //************************ IAddListenerCodec interface starts *******************************************//
                    std::auto_ptr<ClientMessage> encodeRequest() const;

                    std::string decodeResponse(ClientMessage &responseMessage) const;

                    //************************ IAddListenerCodec interface ends *********************************************//
                    private:
                        // Preventing public access to constructors
                        MapAddNearCacheEntryListenerCodec ();

                        std::string name_;
                        int32_t listenerFlags_;
                        bool localOnly_;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_PROTOCOL_CODEC_MAPADDNEARCACHEENTRYLISTENERCODEC_H_ */

