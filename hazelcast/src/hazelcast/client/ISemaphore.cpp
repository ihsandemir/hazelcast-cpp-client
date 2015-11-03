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
#include "hazelcast/client/ISemaphore.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/SemaphoreInitParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreAcquireParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreAvailablePermitsParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreDrainPermitsParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreReducePermitsParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreReleaseParameters.h"
#include "hazelcast/client/protocol/parameters/SemaphoreTryAcquireParameters.h"

namespace hazelcast {
    namespace client {

        ISemaphore::ISemaphore(const std::string &name, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:semaphoreService", name, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&name);
            partitionId = getPartitionId(keyData);
        }

        bool ISemaphore::init(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreInitParameters::encode(getName(), permits);

            return invokeAndGetResult<bool>(request, partitionId);
        }

        void ISemaphore::acquire() {
            acquire(1);
        }

        void ISemaphore::acquire(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreAcquireParameters::encode(getName(), permits);

            invoke(request, partitionId);
        }

        int ISemaphore::availablePermits() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreAvailablePermitsParameters::encode(getName());

            return invokeAndGetResult<int>(request, partitionId);
        }

        int ISemaphore::drainPermits() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreDrainPermitsParameters::encode(getName());

            return invokeAndGetResult<int>(request, partitionId);
        }

        void ISemaphore::reducePermits(int reduction) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreReducePermitsParameters::encode(getName(), reduction);

            invoke(request, partitionId);
        }

        void ISemaphore::release() {
            release(1);
        }

        void ISemaphore::release(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreReleaseParameters::encode(getName(), permits);

            invoke(request, partitionId);
        }

        bool ISemaphore::tryAcquire() {
            return tryAcquire(int(1));
        }

        bool ISemaphore::tryAcquire(int permits) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreTryAcquireParameters::encode(getName(), permits, 0);

            return invokeAndGetResult<bool>(request, partitionId);
        }

        bool ISemaphore::tryAcquire(long timeoutInMillis) {
            return tryAcquire(1, timeoutInMillis);
        }

        bool ISemaphore::tryAcquire(int permits, long timeoutInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::SemaphoreTryAcquireParameters::encode(getName(), permits, timeoutInMillis);

            return invokeAndGetResult<bool>(request, partitionId);
        }
    }
}
