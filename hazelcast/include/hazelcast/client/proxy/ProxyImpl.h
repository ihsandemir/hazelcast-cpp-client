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
//
// Created by sancar koyunlu on 01/10/14.
//


#ifndef HAZELCAST_ProxyImpl
#define HAZELCAST_ProxyImpl

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/parameters/GenericResultParameters.h"
#include "hazelcast/client/protocol/ProtocolTypeDefs.h"

#include <string>

namespace hazelcast {
    namespace client {
        namespace map {
            class DataEntryView;
        }
        namespace connection {
            class Connection;
        }

        namespace impl {
            class BaseEventHandler;

            class BaseRemoveListenerRequest;

            class ClientRequest;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace protocol {
            class ClientMessage;
        }

        namespace proxy {
            class HAZELCAST_API ProxyImpl : public DistributedObject{
            protected:
                typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > EntryVector;

                /**
                * Constructor
                */
                ProxyImpl(const std::string& serviceName, const std::string& objectName, spi::ClientContext *context);

                /**
                * Destructor
                */
                virtual ~ProxyImpl();

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param partitionId that given request will be send to.
                * @param request ClientRequest ptr.
                */
                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request, int partitionId);

                /**
                * Internal API.
                * method to be called by distributed objects.
                * memory ownership is moved to DistributedObject.
                *
                * @param request ClientMessage ptr.
                */
                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request);

                std::auto_ptr<protocol::ClientMessage> invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                              boost::shared_ptr<connection::Connection> conn);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param partitionId
                * @param handler
                */
                std::auto_ptr<std::string> listen(std::auto_ptr<protocol::ClientMessage> registrationRequest, int partitionId,
                                   impl::BaseEventHandler *handler);

                /**
                * Internal API.
                *
                * @param registrationRequest ClientRequest ptr.
                * @param handler
                */
                std::auto_ptr<std::string> listen(std::auto_ptr<protocol::ClientMessage> registrationRequest, impl::BaseEventHandler *handler);

                /**
                * Internal API.
                * @param key
                */
                int getPartitionId(const serialization::pimpl::Data &key);

                template<typename T>
                serialization::pimpl::Data toData(const T& object) {
                    return context->getSerializationService().template toData<T>(&object);
                }

                template<typename T>
                boost::shared_ptr<T> toObject(const serialization::pimpl::Data& data) {
                    return context->getSerializationService().template toObject<T>(data);
                }

                template<typename V>
                std::vector<V> toObjectCollection(std::auto_ptr<protocol::DataArray> collection) {
                    protocol::DataArray &dataCollection = *collection;
                    size_t size = dataCollection.size();
                    std::vector<V> multimap(size);
                    for (size_t i = 0; i < size; i++) {
                        boost::shared_ptr<V> v = toObject<V>(*dataCollection[i]);
                        multimap[i] = *v;
                    }
                    return multimap;
                }

                template<typename T>
                const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T>& elements) {
                    size_t size = elements.size();
                    std::vector<serialization::pimpl::Data> dataCollection(size);
                    for (size_t i = 0; i < size; ++i) {
                        dataCollection[i] = toData(elements[i]);
                    }
                    return dataCollection;
                }

                template <typename K, typename V>
                std::vector<std::pair<K, V> > toObjectEntrySet(const std::auto_ptr<protocol::DataArray> keys,
                                                               const std::auto_ptr<protocol::DataArray> values) {
                    size_t size = keys->size();
                    std::vector<std::pair<K, V> > entrySet(size);
                    for (size_t i = 0; i < size; i++) {
                        boost::shared_ptr<K> key = toObject<K>(*((*keys)[i]));
                        entrySet[i].first = *key;
                        boost::shared_ptr<V> value = toObject<V>(*((*values)[i]));
                        entrySet[i].second = *value;
                    }
                    return entrySet;
                }

                template<typename K, typename V>
                void toDataEntriesSet(std::map<K, V> const &m, std::auto_ptr<protocol::DataArray> &resultKeys,
                                      std::auto_ptr<protocol::DataArray> &resultValues) {

                    unsigned long size = m.size();

                    resultKeys = std::auto_ptr<protocol::DataArray>(new protocol::DataArray(size));
                    resultValues = std::auto_ptr<protocol::DataArray>(new protocol::DataArray(size));

                    for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
                        resultKeys->push_back(new serialization::pimpl::Data(toData<K>(it->first)));
                        resultValues->push_back(new serialization::pimpl::Data(toData<V>(it->second)));
                    }
                }

                template<typename T>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                    return getResponseResult<T>(response);
                }

                template<typename T>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     boost::shared_ptr<connection::Connection> conn) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return getResponseResult<T>(response);
                }

                template<typename T>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, partitionId);

                    return getResponseResult<T>(response);
                }

                template<typename T>
                T invokeAndGetResult(std::auto_ptr<protocol::ClientMessage> request,
                                     int partitionId, boost::shared_ptr<connection::Connection> conn) {
                    std::auto_ptr<protocol::ClientMessage> response = invoke(request, conn);

                    return getResponseResult<T>(response);
                }

                std::auto_ptr<protocol::DataArray> invokeAndGetResult(
                        std::auto_ptr<protocol::ClientMessage> request);

                std::auto_ptr<protocol::DataArray> getDataList(protocol::ClientMessage *response) const;

                spi::ClientContext *context;
            public:
                /**
                * Destroys this object cluster-wide.
                * Clears and releases all resources for this object.
                */
                void destroy();

            private:
                template<typename T>
                inline T getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                    std::auto_ptr<protocol::parameters::GenericResultParameters> resultParameters =
                            protocol::parameters::GenericResultParameters::decode(*response);

                    return *this->context->getSerializationService().toObject<T>(
                            *resultParameters->data);
                }
            };

            template<>
            bool ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            int ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            long ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            std::auto_ptr<std::string> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            std::auto_ptr<serialization::pimpl::Data> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            std::auto_ptr<protocol::DataArray> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);

            template<>
            std::auto_ptr<map::DataEntryView> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response);
        }
    }
}

#endif //HAZELCAST_ProxyImpl
