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
// Created by sancar koyunlu on 30/09/14.
//

#include "hazelcast/client/proxy/IListImpl.h"















#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/impl/PortableCollection.h"
#include "hazelcast/client/impl/SerializableCollection.h"

#include "hazelcast/client/serialization/pimpl/Data.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/ListSizeParameters.h"
#include "hazelcast/client/protocol/parameters/ListContainsParameters.h"
#include "hazelcast/client/protocol/parameters/ListContainsAllParameters.h"
#include "hazelcast/client/protocol/parameters/ListAddParameters.h"
#include "hazelcast/client/protocol/parameters/ListRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/ListAddAllParameters.h"
#include "hazelcast/client/protocol/parameters/ListCompareAndRemoveAllParameters.h"
#include "hazelcast/client/protocol/parameters/ListCompareAndRetainAllParameters.h"
#include "hazelcast/client/protocol/parameters/ListClearParameters.h"
#include "hazelcast/client/protocol/parameters/ListGetAllParameters.h"
#include "hazelcast/client/protocol/parameters/ListAddListenerParameters.h"
#include "hazelcast/client/protocol/parameters/ListRemoveListenerParameters.h"
#include "hazelcast/client/protocol/parameters/ListIsEmptyParameters.h"
#include "hazelcast/client/protocol/parameters/ListAddAllWithIndexParameters.h"
#include "hazelcast/client/protocol/parameters/ListGetParameters.h"
#include "hazelcast/client/protocol/parameters/ListSetParameters.h"
#include "hazelcast/client/protocol/parameters/ListAddWithIndexParameters.h"
#include "hazelcast/client/protocol/parameters/ListRemoveWithIndexParameters.h"
#include "hazelcast/client/protocol/parameters/ListLastIndexOfParameters.h"
#include "hazelcast/client/protocol/parameters/ListIndexOfParameters.h"
#include "hazelcast/client/protocol/parameters/ListSubParameters.h"
#include "hazelcast/client/protocol/parameters/ListIteratorParameters.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            IListImpl::IListImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::auto_ptr<std::string> IListImpl::addItemListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListAddListenerParameters::encode(getName(), includeValue);

                return listen(request, entryEventHandler);
            }

            bool IListImpl::removeItemListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::ListRemoveListenerParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;

            }

            int IListImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListSizeParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }

            bool IListImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListContainsParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IListImpl::toArray() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListGetAllParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool IListImpl::add(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListAddParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListRemoveParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListContainsAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListAddAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::addAll(int index, const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListAddAllWithIndexParameters::encode(getName(), index, elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListCompareAndRemoveAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IListImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListCompareAndRetainAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IListImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListClearParameters::encode(getName());

                invoke(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::get(int index) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListGetParameters::encode(getName(), index);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::set(int index, const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListSetParameters::encode(getName(), index, element);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            void IListImpl::add(int index, const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListAddWithIndexParameters::encode(getName(), index, element);

                invoke(request, partitionId);
            }

            serialization::pimpl::Data IListImpl::remove(int index) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListRemoveWithIndexParameters::encode(getName(), index);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            int IListImpl::indexOf(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListIndexOfParameters::encode(getName(), element);

                return invokeAndGetResult<int>(request, partitionId);
            }

            int IListImpl::lastIndexOf(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListLastIndexOfParameters::encode(getName(), element);

                return invokeAndGetResult<int>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IListImpl::subList(int fromIndex, int toIndex) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::ListSubParameters::encode(getName(), fromIndex, toIndex);

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

        }
    }
}

