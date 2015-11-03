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
#include "hazelcast/client/ICountDownLatch.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/CountDownLatchAwaitParameters.h"
#include "hazelcast/client/protocol/parameters/CountDownLatchCountDownParameters.h"
#include "hazelcast/client/protocol/parameters/CountDownLatchGetCountParameters.h"
#include "hazelcast/client/protocol/parameters/CountDownLatchTrySetCountParameters.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {

        ICountDownLatch::ICountDownLatch(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        bool ICountDownLatch::await(long timeoutInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::CountDownLatchAwaitParameters::encode(getName(), timeoutInMillis);

            return invokeAndGetResult<bool>(request, partitionId);
        }

        void ICountDownLatch::countDown() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::CountDownLatchCountDownParameters::encode(getName());

            invoke(request, partitionId);
        }

        int ICountDownLatch::getCount() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::CountDownLatchGetCountParameters::encode(getName());

            return invokeAndGetResult<int>(request, partitionId);
        }

        bool ICountDownLatch::trySetCount(int count) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::CountDownLatchTrySetCountParameters::encode(getName(), count);

            return invokeAndGetResult<bool>(request, partitionId);
        }
    }
}
