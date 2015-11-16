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
#include "hazelcast/client/protocol/codec/SetSizeCodec.h"
#include "hazelcast/client/protocol/codec/SetContainsCodec.h"
#include "hazelcast/client/protocol/codec/SetContainsAllCodec.h"
#include "hazelcast/client/protocol/codec/SetAddCodec.h"
#include "hazelcast/client/protocol/codec/SetRemoveCodec.h"
#include "hazelcast/client/protocol/codec/SetAddAllCodec.h"
#include "hazelcast/client/protocol/codec/SetCompareAndRemoveAllCodec.h"
#include "hazelcast/client/protocol/codec/SetCompareAndRetainAllCodec.h"
#include "hazelcast/client/protocol/codec/SetClearCodec.h"
#include "hazelcast/client/protocol/codec/SetGetAllCodec.h"
#include "hazelcast/client/protocol/codec/SetAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/SetRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/SetIsEmptyCodec.h"

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
                        protocol::codec::SetAddListenerCodec::RequestParameters::encode(getName(), includeValue);

                return registerListener(request, itemEventHandler);
            }

            bool ISetImpl::removeItemListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::SetRemoveListenerCodec::RequestParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            int ISetImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::SetSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }


            bool ISetImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray>  ISetImpl::toArray() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetGetAllCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool ISetImpl::add(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetRemoveCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetContainsAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetAddAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRemoveAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool ISetImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetCompareAndRetainAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void ISetImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::SetClearCodec::RequestParameters::encode(getName());

                invoke(request, partitionId);
            }
        }
    }
}
