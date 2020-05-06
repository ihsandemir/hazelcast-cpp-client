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

#include "hazelcast/client/spi/ClientProxyFactory.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/map/NearCachedClientMapProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            namespace impl {
                class HAZELCAST_API ClientMapProxyFactory : public spi::ClientProxyFactory {
                public:
                    ClientMapProxyFactory(spi::ClientContext *context) : clientContext(context) {}

                    std::shared_ptr<spi::ClientProxy> create(const std::string &name) override {
                        auto nearCacheConfig = clientContext->getClientConfig().template getNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(
                                name);
                        if (nearCacheConfig != NULL) {
                            //TODO checkNearCacheConfig(nearCacheConfig, true);
                            return std::make_shared<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                                    name, clientContext, *nearCacheConfig);
                        } else {
                            return std::make_shared<map::ClientMapProxy>(name, clientContext);
                        }
                    }
                private:
                    spi::ClientContext *clientContext;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

