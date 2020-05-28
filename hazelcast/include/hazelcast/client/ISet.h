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

#include "hazelcast/client/proxy/ISetImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

namespace hazelcast {
    namespace client {
        /**
        * Concurrent, distributed client implementation of std::unordered_set.
        *
        */
        class HAZELCAST_API ISet : public proxy::ISetImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:setService";

            /**
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            *  @param listener to be added
            *  @param includeValue boolean value representing value should be included in incoming ItemEvent or not.
            *  @returns registrationId that can be used to remove item listener
            */
            template<typename Listener>
            boost::future<std::string> addItemListener(Listener &&listener, bool includeValue) {
                std::unique_ptr<impl::ItemEventHandler<Listener, protocol::codec::SetAddListenerCodec::AbstractEventHandler>> itemEventHandler(
                        new impl::ItemEventHandler<Listener, protocol::codec::SetAddListenerCodec::AbstractEventHandler>(
                                getName(), getContext().getClientClusterService(),
                                getContext().getSerializationService(),
                                listener,
                                includeValue));
                return proxy::ISetImpl::addItemListener(itemEventHandler, includeValue);
            }

            /**
            *
            * @param element to be searched
            * @returns true if set contains element
            */
            template<typename E>
            boost::future<bool> contains(const E &element) {
                return proxy::ISetImpl::contains(toData(element));
            }

            /**
            *
            * @returns all elements as std::vector
            */
            template<typename E>
            boost::future<std::vector<E>> toArray() {
                return toObjectVector<E>(proxy::ISetImpl::toArrayData());
            }

            /**
            *
            * @param element to be added
            * @return true if element is added successfully. If elements was already there returns false.
            */
            template<typename E>
            boost::future<bool> add(const E &element) {
                return proxy::ISetImpl::add(toData(element));
            }

            /**
            *
            * @param element to be removed
            * @return true if element is removed successfully.
            */
            template<typename E>
            boost::future<bool> remove(const E &element) {
                return proxy::ISetImpl::remove(toData(element));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if this set contains all elements given in vector.
            */
            template<typename E>
            boost::future<bool> containsAll(const std::vector<E> &elements) {
                return proxy::ISetImpl::containsAll(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements given in vector can be added to set.
            */
            template<typename E>
            boost::future<bool> addAll(const std::vector<E> &elements) {
                std::vector<serialization::pimpl::Data> dataCollection = toDataCollection(elements);
                return proxy::ISetImpl::addAll(toDataCollection(elements));
            }

            /**
            *
            * @param elements std::vector<E>
            * @return true if all elements are removed successfully.
            */
            template<typename E>
            boost::future<bool> removeAll(const std::vector<E> &elements) {
                return proxy::ISetImpl::removeAll(toDataCollection(elements));
            }

            /**
            *
            * Removes the elements from this set that are not available in given "elements" vector
            * @param elements std::vector<E>
            * @return true if operation is successful.
            */
            template<typename E>
            boost::future<bool> retainAll(const std::vector<E> &elements) {
                return proxy::ISetImpl::retainAll(toDataCollection(elements));
            }

        private:
            ISet(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ISetImpl(instanceName, context) {}
        };
    }
}

