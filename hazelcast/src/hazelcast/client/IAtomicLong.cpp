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
#include "hazelcast/client/IAtomicLong.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/AtomicLongApplyParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongAlterParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongAlterAndGetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongGetAndAlterParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongAddAndGetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongCompareAndSetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongDecrementAndGetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongGetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongGetAndAddParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongGetAndSetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongIncrementAndGetParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongGetAndIncrementParameters.h"
#include "hazelcast/client/protocol/parameters/AtomicLongSetParameters.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {


        IAtomicLong::IAtomicLong(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        long IAtomicLong::addAndGet(long delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::AtomicLongAddAndGetParameters::encode(getName(), delta);

            return invokeAndGetResult<long>(request, partitionId);
        }

        bool IAtomicLong::compareAndSet(long expect, long update) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::AtomicLongCompareAndSetParameters::encode(getName(), expect, update);

            return invokeAndGetResult<bool>(request, partitionId);
        }

        long IAtomicLong::decrementAndGet() {
            return addAndGet(-1);
        }

        long IAtomicLong::get() {
            return getAndAdd(0);
        }

        long IAtomicLong::getAndAdd(long delta) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::AtomicLongGetAndAddParameters::encode(getName(), delta);

            return invokeAndGetResult<long>(request, partitionId);
        }

        long IAtomicLong::getAndSet(long newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::AtomicLongGetAndSetParameters::encode(getName(), newValue);

            return invokeAndGetResult<long>(request, partitionId);
        }

        long IAtomicLong::incrementAndGet() {
            return addAndGet(1);
        }

        long IAtomicLong::getAndIncrement() {
            return getAndAdd(1);
        }

        void IAtomicLong::set(long newValue) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::AtomicLongSetParameters::encode(getName(), newValue);

            invoke(request, partitionId);
        }
    }
}
