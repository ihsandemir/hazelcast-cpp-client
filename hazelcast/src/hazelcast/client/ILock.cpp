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
// Created by sancar koyunlu on 6/27/13.



#include "hazelcast/client/ILock.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/LockIsLockedParameters.h"
#include "hazelcast/client/protocol/parameters/LockIsLockedByCurrentThreadParameters.h"
#include "hazelcast/client/protocol/parameters/LockGetLockCountParameters.h"
#include "hazelcast/client/protocol/parameters/LockGetRemainingLeaseTimeParameters.h"
#include "hazelcast/client/protocol/parameters/LockLockParameters.h"
#include "hazelcast/client/protocol/parameters/LockUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/LockForceUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/LockTryLockParameters.h"

#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        ILock::ILock(const std::string& instanceName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:lockService", instanceName, context)
        , key(toData<std::string>(instanceName)) {
            partitionId = getPartitionId(key);
        }

        void ILock::lock() {
            lock(-1);
        }

        void ILock::lock(long leaseTimeInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockLockParameters::encode(getName(), leaseTimeInMillis, util::getThreadId());

            invoke(request, partitionId);
        }

        void ILock::unlock() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockUnlockParameters::encode(getName(), util::getThreadId());

            invoke(request, partitionId);
        }

        void ILock::forceUnlock() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockForceUnlockParameters::encode(getName());

            invoke(request, partitionId);
        }

        bool ILock::isLocked() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockIsLockedParameters::encode(getName());

            return invokeAndGetResult<bool>(request, partitionId);
        }

        bool ILock::isLockedByCurrentThread() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockIsLockedByCurrentThreadParameters::encode(getName(), util::getThreadId());

            return invokeAndGetResult<bool>(request, partitionId);
        }

        int ILock::getLockCount() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockGetLockCountParameters::encode(getName());

            return invokeAndGetResult<int>(request, partitionId);
        }

        long ILock::getRemainingLeaseTime() {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockGetRemainingLeaseTimeParameters::encode(getName());

            return invokeAndGetResult<long>(request, partitionId);
        }

        bool ILock::tryLock() {
            return tryLock(0);
        }

        bool ILock::tryLock(long timeInMillis) {
            std::auto_ptr<protocol::ClientMessage> request =
                    protocol::parameters::LockTryLockParameters::encode(getName(), util::getThreadId(), timeInMillis);

            return invokeAndGetResult<bool>(request, partitionId);
        }
    }
}
