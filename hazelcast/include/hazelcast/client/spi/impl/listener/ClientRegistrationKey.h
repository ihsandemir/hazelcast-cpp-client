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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_CLIENTREGISTRATIONKEY_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_CLIENTREGISTRATIONKEY_H_

#include <string>
#include <memory>
#include <ostream>
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class BaseEventHandler;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            namespace impl {
                class ListenerMessageCodec;

                namespace listener {
                    class HAZELCAST_API ClientRegistrationKey {
                    public:
                        ClientRegistrationKey();

                        ClientRegistrationKey(const std::string &userRegistrationId,
                                              std::unique_ptr<client::impl::BaseEventHandler> &&handler,
                                              std::unique_ptr<ListenerMessageCodec> &&codec);

                        ClientRegistrationKey(const std::string &userRegistrationId);

                        const std::string &getUserRegistrationId() const;

                        const std::shared_ptr<client::impl::BaseEventHandler> &getHandler() const;

                        const std::shared_ptr<ListenerMessageCodec> &getCodec() const;

                        bool operator==(const ClientRegistrationKey &rhs) const;

                        bool operator!=(const ClientRegistrationKey &rhs) const;

                        bool operator<(const ClientRegistrationKey &rhs) const;

                        friend std::ostream &operator<<(std::ostream &os, const ClientRegistrationKey &key);

                    private:
                        std::string userRegistrationId;
                        std::shared_ptr<client::impl::BaseEventHandler> handler;
                        std::shared_ptr<ListenerMessageCodec> codec;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_CLIENTREGISTRATIONKEY_H_
