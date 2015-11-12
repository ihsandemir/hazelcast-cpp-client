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

#include "hazelcast/client/proxy/ISetImpl.h"

#include "hazelcast/client/impl/ItemEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/SerializableCollection.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/SetSizeParameters.h"
#include "hazelcast/client/protocol/parameters/SetContainsParameters.h"
#include "hazelcast/client/protocol/parameters/SetContainsAllParameters.h"
#include "hazelcast/client/protocol/parameters/SetAddParameters.h"
#include "hazelcast/client/protocol/parameters/SetRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/SetAddAllParameters.h"
#include "hazelcast/client/protocol/parameters/SetCompareAndRemoveAllParameters.h"
#include "hazelcast/client/protocol/parameters/SetCompareAndRetainAllParameters.h"
#include "hazelcast/client/protocol/parameters/SetClearParameters.h"
#include "hazelcast/client/protocol/parameters/SetGetAllParameters.h"
#include "hazelcast/client/protocol/parameters/SetAddListenerParameters.h"
#include "hazelcast/client/protocol/parameters/SetRemoveListenerParameters.h"
#include "hazelcast/client/protocol/parameters/SetIsEmptyParameters.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            ISetImpl::ISetImpl(const std::string& instanceName, spi::ClientContext *clientContext)
            : ProxyImpl("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(keyData);
            }

            std::auto_ptr<std::string> ISetImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetAddListenerParameters::encode(getName(), includeValue);

                return registerListener(request, itemEventHandler);
            }

            bool ISetImpl::removeItemListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::SetRemoveListenerParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            int ISetImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::SetSizeParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }


            bool ISetImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetContainsParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray>  ISetImpl::toArray() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetGetAllParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool ISetImpl::add(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetAddParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetRemoveParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetContainsAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetAddAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetCompareAndRemoveAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetCompareAndRetainAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void ISetImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::SetClearParameters::encode(getName());

                invoke(request, partitionId);
            }
        }
    }
}
