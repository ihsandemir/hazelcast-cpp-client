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
#include "hazelcast/client/protocol/parameters/QueueOfferParameters.h"
#include "hazelcast/client/protocol/parameters/QueuePutParameters.h"
#include "hazelcast/client/protocol/parameters/QueueSizeParameters.h"
#include "hazelcast/client/protocol/parameters/QueueRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/QueuePollParameters.h"
#include "hazelcast/client/protocol/parameters/QueueTakeParameters.h"
#include "hazelcast/client/protocol/parameters/QueuePeekParameters.h"
#include "hazelcast/client/protocol/parameters/QueueIteratorParameters.h"
#include "hazelcast/client/protocol/parameters/QueueDrainToParameters.h"
#include "hazelcast/client/protocol/parameters/QueueDrainToMaxSizeParameters.h"
#include "hazelcast/client/protocol/parameters/QueueContainsParameters.h"
#include "hazelcast/client/protocol/parameters/QueueContainsAllParameters.h"
#include "hazelcast/client/protocol/parameters/QueueCompareAndRemoveAllParameters.h"
#include "hazelcast/client/protocol/parameters/QueueCompareAndRetainAllParameters.h"
#include "hazelcast/client/protocol/parameters/QueueClearParameters.h"
#include "hazelcast/client/protocol/parameters/QueueAddAllParameters.h"
#include "hazelcast/client/protocol/parameters/QueueAddListenerParameters.h"
#include "hazelcast/client/protocol/parameters/QueueRemoveListenerParameters.h"
#include "hazelcast/client/protocol/parameters/QueueRemainingCapacityParameters.h"
#include "hazelcast/client/protocol/parameters/QueueIsEmptyParameters.h"

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
                        protocol::parameters::QueueAddListenerParameters::encode(getName(), includeValue);

                return registerListener(request, itemEventHandler);
            }

            bool IQueueImpl::removeItemListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::QueueRemoveListenerParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            bool IQueueImpl::offer(const serialization::pimpl::Data& element, long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueOfferParameters::encode(getName(), element, timeoutInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            serialization::pimpl::Data IQueueImpl::poll(long timeoutInMillis) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueuePollParameters::encode(getName(), timeoutInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueRemainingCapacityParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }

            bool IQueueImpl::remove(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueRemoveParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::contains(const serialization::pimpl::Data& element) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueContainsParameters::encode(getName(), element);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IQueueImpl::drainTo(int maxElements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueDrainToMaxSizeParameters::encode(getName(), maxElements);

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }


            serialization::pimpl::Data IQueueImpl::peek() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueuePeekParameters::encode(getName());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            int IQueueImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueSizeParameters::encode(getName());

                return invokeAndGetResult<int>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> IQueueImpl::toArray() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueIteratorParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueContainsAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::addAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueAddAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueCompareAndRemoveAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data>& elements) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueCompareAndRetainAllParameters::encode(getName(), elements);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IQueueImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::QueueClearParameters::encode(getName());

                invoke(request, partitionId);
            }

        }
    }
}

