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

#include "hazelcast/client/proxy/IQueueImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/QueueOfferCodec.h"
#include "hazelcast/client/protocol/codec/QueuePutCodec.h"
#include "hazelcast/client/protocol/codec/QueueSizeCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemoveCodec.h"
#include "hazelcast/client/protocol/codec/QueuePollCodec.h"
#include "hazelcast/client/protocol/codec/QueueTakeCodec.h"
#include "hazelcast/client/protocol/codec/QueuePeekCodec.h"
#include "hazelcast/client/protocol/codec/QueueIteratorCodec.h"
#include "hazelcast/client/protocol/codec/QueueDrainToCodec.h"
#include "hazelcast/client/protocol/codec/QueueDrainToMaxSizeCodec.h"
#include "hazelcast/client/protocol/codec/QueueContainsCodec.h"
#include "hazelcast/client/protocol/codec/QueueContainsAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueCompareAndRemoveAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueCompareAndRetainAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueClearCodec.h"
#include "hazelcast/client/protocol/codec/QueueAddAllCodec.h"
#include "hazelcast/client/protocol/codec/QueueAddListenerCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemoveListenerCodec.h"
#include "hazelcast/client/protocol/codec/QueueRemainingCapacityCodec.h"
#include "hazelcast/client/protocol/codec/QueueIsEmptyCodec.h"

#include "hazelcast/client/spi/ServerListenerService.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            IQueueImpl::IQueueImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = context->getSerializationService().toData<std::string>(&instanceName);
                partitionId = getPartitionId(data);
            }

            std::auto_ptr<std::string> IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueAddListenerCodec::RequestParameters::encode(getName(), includeValue);

                return registerListener(request, itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::QueueRemoveListenerCodec::RequestParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data& element, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueOfferCodec::RequestParameters::encode(getName(), element, timeoutInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            serialization::pimpl::Data IQueueImpl::poll(long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePollCodec::RequestParameters::encode(getName(), timeoutInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemainingCapacityCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueRemoveCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsCodec::RequestParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IQueueImpl::drainTo(int maxElements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueDrainToMaxSizeCodec::RequestParameters::encode(getName(), maxElements);

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }


            serialization::pimpl::Data IQueueImpl::peek() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueuePeekCodec::RequestParameters::encode(getName());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            int IQueueImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IQueueImpl::toArray() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueIteratorCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueContainsAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueAddAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRemoveAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueCompareAndRetainAllCodec::RequestParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IQueueImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::QueueClearCodec::RequestParameters::encode(getName());

                invoke(request, partitionId);
            }

        }
    }
}

