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

#include <limits.h>

#include "hazelcast/client/impl/ClientLockReferenceIdGenerator.h"
#include "hazelcast/client/crdt/pncounter/impl/PNCounterProxyFactory.h"
#include "hazelcast/client/proxy/ClientPNCounterProxy.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/proxy/ClientFlakeIdGeneratorProxy.h"
#include "hazelcast/client/flakeidgen/impl/FlakeIdGeneratorProxyFactory.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"
#include "hazelcast/client/proxy/ClientRingbufferProxy.h"
#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/internal/partition/strategy/StringPartitioningStrategy.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/TimeUtil.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            ClientLockReferenceIdGenerator::ClientLockReferenceIdGenerator() : referenceIdCounter(0) {}

            int64_t ClientLockReferenceIdGenerator::getNextReferenceId() {
                return ++referenceIdCounter;
            }
        }

        namespace crdt {
            namespace pncounter {
                namespace impl {
                    PNCounterProxyFactory::PNCounterProxyFactory(spi::ClientContext *clientContext) : clientContext(
                            clientContext) {}

                    std::shared_ptr<spi::ClientProxy> PNCounterProxyFactory::create(const std::string &id) {
                        return std::shared_ptr<spi::ClientProxy>(
                                new proxy::ClientPNCounterProxy(proxy::ClientPNCounterProxy::SERVICE_NAME, id,
                                                                clientContext));
                    }
                }
            }
        }

        namespace flakeidgen {
            namespace impl {
                FlakeIdGeneratorProxyFactory::FlakeIdGeneratorProxyFactory(spi::ClientContext *clientContext)
                        : clientContext(
                        clientContext) {}

                std::shared_ptr<spi::ClientProxy> FlakeIdGeneratorProxyFactory::create(const std::string &id) {
                    return std::shared_ptr<spi::ClientProxy>(
                            new proxy::ClientFlakeIdGeneratorProxy(id, clientContext));
                }


                AutoBatcher::AutoBatcher(int32_t batchSize, int64_t validity,
                                         const std::shared_ptr<AutoBatcher::IdBatchSupplier> &batchIdSupplier)
                        : batchSize(batchSize), validity(validity), batchIdSupplier(batchIdSupplier),
                          block(std::shared_ptr<Block>(new Block(IdBatch(0, 0, 0), 0))) {}

                int64_t AutoBatcher::newId() {
                    for (;;) {
                        std::shared_ptr<Block> b = this->block;
                        int64_t res = b->next();
                        if (res != INT64_MIN) {
                            return res;
                        }

                        {
                            std::lock_guard<std::mutex> guard(lock);
                            if (b != this->block.get()) {
                                // new block was assigned in the meantime
                                continue;
                            }
                            this->block = std::shared_ptr<Block>(
                                    new Block(batchIdSupplier->newIdBatch(batchSize), validity));
                        }
                    }
                }

                AutoBatcher::Block::Block(const IdBatch &idBatch, int64_t validity) : idBatch(idBatch), numReturned(0) {
                    invalidSince = validity > 0 ? util::currentTimeMillis() + validity : INT64_MAX;
                }

                int64_t AutoBatcher::Block::next() {
                    if (invalidSince <= util::currentTimeMillis()) {
                        return INT64_MIN;
                    }
                    int32_t index;
                    do {
                        index = numReturned;
                        if (index == idBatch.getBatchSize()) {
                            return INT64_MIN;
                        }
                    } while (!numReturned.compare_exchange_strong(index, index + 1));

                    return idBatch.getBase() + index * idBatch.getIncrement();
                }

                IdBatch::IdIterator IdBatch::endOfBatch;

                const int64_t IdBatch::getBase() const {
                    return base;
                }

                const int64_t IdBatch::getIncrement() const {
                    return increment;
                }

                const int32_t IdBatch::getBatchSize() const {
                    return batchSize;
                }

                IdBatch::IdBatch(const int64_t base, const int64_t increment, const int32_t batchSize)
                        : base(base), increment(increment), batchSize(batchSize) {}

                IdBatch::IdIterator &IdBatch::end() {
                    return endOfBatch;
                }

                IdBatch::IdIterator IdBatch::iterator() {
                    return IdBatch::IdIterator(base, increment, batchSize);
                }

                IdBatch::IdIterator::IdIterator(int64_t base2, const int64_t increment, int32_t remaining) : base2(
                        base2), increment(increment), remaining(remaining) {}

                bool IdBatch::IdIterator::operator==(const IdBatch::IdIterator &rhs) const {
                    return base2 == rhs.base2 && increment == rhs.increment && remaining == rhs.remaining;
                }

                bool IdBatch::IdIterator::operator!=(const IdBatch::IdIterator &rhs) const {
                    return !(rhs == *this);
                }

                IdBatch::IdIterator::IdIterator() : base2(-1), increment(-1), remaining(-1) {
                }

                IdBatch::IdIterator &IdBatch::IdIterator::operator++() {
                    if (remaining == 0) {
                        return IdBatch::end();
                    }

                    --remaining;

                    base2 += increment;

                    return *this;
                }

            }
        }

        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:multiMapService", instanceName, context) {
                // TODO: remove this line once the client instance getDistributedObject works as expected in Java for this proxy type
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            boost::future<bool> MultiMapImpl::put(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MultiMapPutCodec::encodeRequest(getName(), key, value,
                                                                         util::getCurrentThreadId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapPutCodec::ResponseParameters>(request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::getData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapGetCodec::encodeRequest(getName(), key,
                                                                         util::getCurrentThreadId());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapGetCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<bool> MultiMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MultiMapRemoveEntryCodec::encodeRequest(getName(), key, value,
                                                                                 util::getCurrentThreadId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapRemoveEntryCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::removeData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapRemoveCodec::encodeRequest(getName(), key,
                                                                            util::getCurrentThreadId());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::keySetData() {
                auto request = protocol::codec::MultiMapKeySetCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapKeySetCodec::ResponseParameters>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> MultiMapImpl::valuesData() {
                auto request = protocol::codec::MultiMapValuesCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MultiMapValuesCodec::ResponseParameters>(
                        request);
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >
            MultiMapImpl::entrySetData() {
                auto request = protocol::codec::MultiMapEntrySetCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >, protocol::codec::MultiMapEntrySetCodec::ResponseParameters>(
                        request);
            }

            boost::future<bool> MultiMapImpl::containsKey(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapContainsKeyCodec::encodeRequest(getName(), key,
                                                                                 util::getCurrentThreadId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapContainsKeyCodec::ResponseParameters>(request,
                                                                                                               key);
            }

            boost::future<bool> MultiMapImpl::containsValue(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MultiMapContainsValueCodec::encodeRequest(getName(), value);

                return invokeAndGetFuture<bool, protocol::codec::MultiMapContainsValueCodec::ResponseParameters>(
                        request);
            }

            boost::future<bool> MultiMapImpl::containsEntry(const serialization::pimpl::Data &key,
                                             const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MultiMapContainsEntryCodec::encodeRequest(getName(), key, value,
                                                                                   util::getCurrentThreadId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapContainsEntryCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<int> MultiMapImpl::size() {
                auto request = protocol::codec::MultiMapSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetFuture<int, protocol::codec::MultiMapSizeCodec::ResponseParameters>(request);
            }

            boost::future<void> MultiMapImpl::clear() {
                auto request = protocol::codec::MultiMapClearCodec::encodeRequest(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapValueCountCodec::encodeRequest(getName(), key,
                                                                                util::getCurrentThreadId());

                return invokeAndGetFuture<int, protocol::codec::MultiMapValueCountCodec::ResponseParameters>(request,
                                                                                                             key);
            }

            boost::future<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue), entryEventHandler);
            }

            boost::future<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                       serialization::pimpl::Data &key, bool includeValue) {
                return registerListener(createMultiMapEntryListenerCodec(includeValue, key), entryEventHandler);
            }

            boost::future<bool> MultiMapImpl::removeEntryListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key) {
                lock(key, -1);
            }

            boost::future<void> MultiMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                auto request = protocol::codec::MultiMapLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                          leaseTime,
                                                                          lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }


            boost::future<bool> MultiMapImpl::isLocked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapIsLockedCodec::encodeRequest(getName(), key);

                return invokeAndGetFuture<bool, protocol::codec::MultiMapIsLockedCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            boost::future<bool> MultiMapImpl::tryLock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key,
                                                                             util::getCurrentThreadId(), LONG_MAX,
                                                                             0,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            boost::future<bool> MultiMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                auto request = protocol::codec::MultiMapTryLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                             LONG_MAX, timeInMillis,
                                                                             lockReferenceIdGenerator->getNextReferenceId());

                return invokeAndGetFuture<bool, protocol::codec::MultiMapTryLockCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            boost::future<void> MultiMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                auto request = protocol::codec::MultiMapUnlockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                            lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            boost::future<void> MultiMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                auto request = protocol::codec::MultiMapForceUnlockCodec::encodeRequest(getName(), key,
                                                                                 lockReferenceIdGenerator->getNextReferenceId());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerMessageCodec(getName(), includeValue));
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            MultiMapImpl::createMultiMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &key) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new MultiMapEntryListenerToKeyCodec(getName(), includeValue, key));
            }

            void MultiMapImpl::onInitialize() {
                ProxyImpl::onInitialize();

                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            MultiMapImpl::MultiMapEntryListenerMessageCodec::MultiMapEntryListenerMessageCodec(const std::string &name,
                                                                                               bool includeValue)
                    : name(name),
                      includeValue(
                              includeValue) {
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            boost::future<std::string> MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::encodeRequest(name, key, includeValue,
                                                                                          localOnly);
            }

            boost::future<std::string> MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MultiMapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            MultiMapImpl::MultiMapEntryListenerToKeyCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool MultiMapImpl::MultiMapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }

            MultiMapImpl::MultiMapEntryListenerToKeyCodec::MultiMapEntryListenerToKeyCodec(const std::string &name,
                                                                                           bool includeValue,
                                                                                           serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), key(key) {}


            std::string ReliableTopicImpl::TOPIC_RB_PREFIX = "_hz_rb_";

            ReliableTopicImpl::ReliableTopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context),
                      logger(context->getLogger()),
                      config(context->getClientConfig().getReliableTopicConfig(instanceName)) {
                ringbuffer = context->getHazelcastClientImplementation()->getRingbuffer<topic::impl::reliable::ReliableTopicMessage>(TOPIC_RB_PREFIX + name);
            }

            boost::future<void> ReliableTopicImpl::publish(const serialization::pimpl::Data &data) {
                std::unique_ptr<Address> nullAddress;
                topic::impl::reliable::ReliableTopicMessage message(data, nullAddress);
                ringbuffer->add(message);
            }

            const std::string ClientPNCounterProxy::SERVICE_NAME = "hz:impl:PNCounterService";
            const std::shared_ptr<std::set<Address> > ClientPNCounterProxy::EMPTY_ADDRESS_LIST(
                    new std::set<Address>());

            ClientPNCounterProxy::ClientPNCounterProxy(const std::string &serviceName, const std::string &objectName,
                                                       spi::ClientContext *context)
                    : ProxyImpl(serviceName, objectName, context), maxConfiguredReplicaCount(0),
                      observedClock(std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock())),
                      logger(context->getLogger()) {
            }

            std::ostream &operator<<(std::ostream &os, const ClientPNCounterProxy &proxy) {
                os << "PNCounter{name='" << proxy.getName() << "\'}";
                return os;
            }

            int64_t ClientPNCounterProxy::get() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::get",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeGetInternal(EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);
                protocol::codec::PNCounterGetCodec::ResponseParameters resultParameters = protocol::codec::PNCounterGetCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndAdd(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndAdd",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(delta, true, EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::addAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(exception::NoDataMemberInClusterException("ClientPNCounterProxy::addAndGet",
                                                                                    "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(delta, false,
                                                  EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndSubtract(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndSubtract",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(-delta, true,
                                                  EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::subtractAndGet(int64_t delta) {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::subtractAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(-delta, false,
                                                  EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::decrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::decrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(-1, false, EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::incrementAndGet() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::incrementAndGet",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(1, false, EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndDecrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndDecrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(-1, true, EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            int64_t ClientPNCounterProxy::getAndIncrement() {
                std::shared_ptr<Address> target = getCRDTOperationTarget(*EMPTY_ADDRESS_LIST);
                if (target.get() == NULL) {
                    BOOST_THROW_EXCEPTION(
                            exception::NoDataMemberInClusterException("ClientPNCounterProxy::getAndIncrement",
                                                                      "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                }
                auto response = invokeAddInternal(1, true, EMPTY_ADDRESS_LIST,
                                                  nullptr,
                                                  target);

                protocol::codec::PNCounterAddCodec::ResponseParameters resultParameters = protocol::codec::PNCounterAddCodec::ResponseParameters::decode(
                        response);
                updateObservedReplicaTimestamps(resultParameters.replicaTimestamps);
                return resultParameters.value;
            }

            void ClientPNCounterProxy::reset() {
                observedClock = std::shared_ptr<cluster::impl::VectorClock>(new cluster::impl::VectorClock());
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::getCRDTOperationTarget(const std::set<Address> &excludedAddresses) {
                if (currentTargetReplicaAddress.get().get() != NULL &&
                    excludedAddresses.find(*currentTargetReplicaAddress.get()) == excludedAddresses.end()) {
                    return currentTargetReplicaAddress;
                }

                {
                    std::lock_guard<std::mutex> guard(targetSelectionMutex);
                    if (currentTargetReplicaAddress.get() == NULL ||
                        excludedAddresses.find(*currentTargetReplicaAddress.get()) != excludedAddresses.end()) {
                        currentTargetReplicaAddress = chooseTargetReplica(excludedAddresses);
                    }
                }
                return currentTargetReplicaAddress;
            }

            std::shared_ptr<Address>
            ClientPNCounterProxy::chooseTargetReplica(const std::set<Address> &excludedAddresses) {
                std::vector<Address> replicaAddresses = getReplicaAddresses(excludedAddresses);
                if (replicaAddresses.empty()) {
                    return std::shared_ptr<Address>();
                }
                // TODO: Use a random generator as used in Java (ThreadLocalRandomProvider) which is per thread
                int randomReplicaIndex = std::abs(rand()) % (int) replicaAddresses.size();
                return std::shared_ptr<Address>(new Address(replicaAddresses[randomReplicaIndex]));
            }

            std::vector<Address> ClientPNCounterProxy::getReplicaAddresses(const std::set<Address> &excludedAddresses) {
                std::vector<Member> dataMembers = getContext().getClientClusterService().getMembers(
                        *cluster::memberselector::MemberSelectors::DATA_MEMBER_SELECTOR);
                int32_t replicaCount = getMaxConfiguredReplicaCount();
                int currentReplicaCount = util::min<int>(replicaCount, (int) dataMembers.size());

                std::vector<Address> replicaAddresses;
                for (int i = 0; i < currentReplicaCount; i++) {
                    const Address &dataMemberAddress = dataMembers[i].getAddress();
                    if (excludedAddresses.find(dataMemberAddress) == excludedAddresses.end()) {
                        replicaAddresses.push_back(dataMemberAddress);
                    }
                }
                return replicaAddresses;
            }

            int32_t ClientPNCounterProxy::getMaxConfiguredReplicaCount() {
                if (maxConfiguredReplicaCount > 0) {
                    return maxConfiguredReplicaCount;
                } else {
                    auto request = protocol::codec::PNCounterGetConfiguredReplicaCountCodec::encodeRequest(
                            getName());
                    maxConfiguredReplicaCount = invokeAndGetFuture<int32_t, protocol::codec::PNCounterGetConfiguredReplicaCountCodec::ResponseParameters>(
                            request);
                }
                return maxConfiguredReplicaCount;
            }

            protocol::ClientMessage
            ClientPNCounterProxy::invokeGetInternal(std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    std::exception_ptr lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException) {
                        std::rethrow_exception(lastException);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }
                try {
                    auto request = protocol::codec::PNCounterGetCodec::encodeRequest(
                            getName(), observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Exception occurred while invoking operation on target ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    return invokeGetInternal(excludedAddresses, std::current_exception(), newTarget);
                }
            }


            protocol::ClientMessage
            ClientPNCounterProxy::invokeAddInternal(int64_t delta, bool getBeforeUpdate,
                                                    std::shared_ptr<std::set<Address> > excludedAddresses,
                                                    std::exception_ptr lastException,
                                                    const std::shared_ptr<Address> &target) {
                if (target.get() == NULL) {
                    if (lastException) {
                        std::rethrow_exception(lastException);
                    } else {
                        BOOST_THROW_EXCEPTION(
                                exception::NoDataMemberInClusterException("ClientPNCounterProxy::invokeGetInternal",
                                                                          "Cannot invoke operations on a CRDT because the cluster does not contain any data members"));
                    }
                }

                try {
                    auto request = protocol::codec::PNCounterAddCodec::encodeRequest(
                            getName(), delta, getBeforeUpdate, observedClock.get()->entrySet(), *target);
                    return invokeOnAddress(request, *target);
                } catch (exception::HazelcastException &e) {
                    logger.finest("Unable to provide session guarantees when sending operations to ", *target,
                                  ", choosing different target. Cause: ", e);
                    if (excludedAddresses == EMPTY_ADDRESS_LIST) {
                        // TODO: Make sure that this only affects the local variable of the method
                        excludedAddresses = std::shared_ptr<std::set<Address> >(new std::set<Address>());
                    }
                    excludedAddresses->insert(*target);
                    std::shared_ptr<Address> newTarget = getCRDTOperationTarget(*excludedAddresses);
                    return invokeAddInternal(delta, getBeforeUpdate, excludedAddresses, std::current_exception(),
                                             newTarget);
                }
            }

            void ClientPNCounterProxy::updateObservedReplicaTimestamps(
                    const std::vector<std::pair<std::string, int64_t> > &receivedLogicalTimestamps) {
                std::shared_ptr<cluster::impl::VectorClock> received = toVectorClock(receivedLogicalTimestamps);
                for (;;) {
                    std::shared_ptr<cluster::impl::VectorClock> currentClock = this->observedClock;
                    if (currentClock->isAfter(*received)) {
                        break;
                    }
                    if (observedClock.compareAndSet(currentClock, received)) {
                        break;
                    }
                }
            }

            std::shared_ptr<cluster::impl::VectorClock> ClientPNCounterProxy::toVectorClock(
                    const std::vector<std::pair<std::string, int64_t> > &replicaLogicalTimestamps) {
                return std::shared_ptr<cluster::impl::VectorClock>(
                        new cluster::impl::VectorClock(replicaLogicalTimestamps));
            }

            std::shared_ptr<Address> ClientPNCounterProxy::getCurrentTargetReplicaAddress() {
                return currentTargetReplicaAddress.get();
            }

            IListImpl::IListImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:listService", instanceName, context) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            boost::future<std::string>
            IListImpl::addItemListener(std::unique_ptr<impl::BaseEventHandler> &&itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), std::move(itemEventHandler));
            }

            boost::future<bool> IListImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<int> IListImpl::size() {
                auto request = protocol::codec::ListSizeCodec::encodeRequest(getName());
                return invokeAndGetFuture<int, protocol::codec::ListSizeCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            boost::future<bool> IListImpl::isEmpty() {
                auto request = protocol::codec::ListIsEmptyCodec::encodeRequest(getName());
                return invokeAndGetFuture<bool, protocol::codec::ListIsEmptyCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            boost::future<bool> IListImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListContainsCodec::encodeRequest(getName(), element);
                return invokeAndGetFuture<bool, protocol::codec::ListContainsCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::toArrayData() {
                auto request = protocol::codec::ListGetAllCodec::encodeRequest(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::ListGetAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> IListImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListAddCodec::encodeRequest(getName(), element);
                return invokeAndGetFuture<bool, protocol::codec::ListAddCodec::ResponseParameters>(request,
                                                                                                   partitionId);
            }

            boost::future<bool> IListImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListRemoveCodec::encodeRequest(getName(), element);
                return invokeAndGetFuture<bool, protocol::codec::ListRemoveCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            boost::future<bool> IListImpl::containsAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::ListContainsAllCodec::encodeRequest(getName(), elements);
                return invokeAndGetFuture<bool, protocol::codec::ListContainsAllCodec::ResponseParameters>(request,
                                                                                                           partitionId);
            }

            boost::future<bool> IListImpl::addAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::ListAddAllCodec::encodeRequest(getName(), elements);
                return invokeAndGetFuture<bool, protocol::codec::ListAddAllCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            boost::future<bool> IListImpl::addAllData(int index, const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::ListAddAllWithIndexCodec::encodeRequest(getName(), index,
                                                                                 elements);
                return invokeAndGetFuture<bool, protocol::codec::ListAddAllWithIndexCodec::ResponseParameters>(request,
                                                                                                               partitionId);
            }

            boost::future<bool> IListImpl::removeAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::ListCompareAndRemoveAllCodec::encodeRequest(getName(), elements);
                return invokeAndGetFuture<bool, protocol::codec::ListCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> IListImpl::retainAllData(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::ListCompareAndRetainAllCodec::encodeRequest(getName(), elements);
                return invokeAndGetFuture<bool, protocol::codec::ListCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<void> IListImpl::clear() {
                auto request = protocol::codec::ListClearCodec::encodeRequest(getName());
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<serialization::pimpl::Data> IListImpl::getData(int index) {
                auto request = protocol::codec::ListGetCodec::encodeRequest(getName(), index);
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::ListGetCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<serialization::pimpl::Data> IListImpl::setData(int index,
                                                                           const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListSetCodec::encodeRequest(getName(), index, element);
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::ListSetCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<void> IListImpl::add(int index, const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListAddWithIndexCodec::encodeRequest(getName(), index, element);
                return toVoidFuture(invokeOnPartition(request, partitionId));
            }

            boost::future<serialization::pimpl::Data> IListImpl::removeData(int index) {
                auto request = protocol::codec::ListRemoveWithIndexCodec::encodeRequest(getName(), index);
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::ListRemoveWithIndexCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<int> IListImpl::indexOf(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListIndexOfCodec::encodeRequest(getName(), element);
                return invokeAndGetFuture<int, protocol::codec::ListIndexOfCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            boost::future<int> IListImpl::lastIndexOf(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::ListLastIndexOfCodec::encodeRequest(getName(), element);
                return invokeAndGetFuture<int, protocol::codec::ListLastIndexOfCodec::ResponseParameters>(request,
                                                                                                          partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IListImpl::subListData(int fromIndex, int toIndex) {
                auto request = protocol::codec::ListSubCodec::encodeRequest(getName(), fromIndex, toIndex);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::ListSubCodec::ResponseParameters>(
                        request, partitionId);
            }

            std::unique_ptr<spi::impl::ListenerMessageCodec> IListImpl::createItemListenerCodec(bool includeValue) {
                return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                        new ListListenerMessageCodec(getName(), includeValue));
            }

            IListImpl::ListListenerMessageCodec::ListListenerMessageCodec(const std::string &name,
                                                                          bool includeValue) : name(name),
                                                                                               includeValue(
                                                                                                       includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            IListImpl::ListListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::ListAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string IListImpl::ListListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::ListAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IListImpl::ListListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::ListRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IListImpl::ListListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::ListRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            const std::string ClientFlakeIdGeneratorProxy::SERVICE_NAME = "hz:impl:flakeIdGeneratorService";

            ClientFlakeIdGeneratorProxy::ClientFlakeIdGeneratorProxy(const std::string &objectName,
                                                                     spi::ClientContext *context)
                    : ProxyImpl(SERVICE_NAME, objectName, context) {
                std::shared_ptr<config::ClientFlakeIdGeneratorConfig> config = getContext().getClientConfig().findFlakeIdGeneratorConfig(
                        getName());
                batcher.reset(
                        new flakeidgen::impl::AutoBatcher(config->getPrefetchCount(),
                                                          config->getPrefetchValidityMillis(),
                                                          std::shared_ptr<flakeidgen::impl::AutoBatcher::IdBatchSupplier>(
                                                                  new FlakeIdBatchSupplier(*this))));
            }

            int64_t ClientFlakeIdGeneratorProxy::newId() {
                return batcher->newId();
            }

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::newIdBatch(int32_t batchSize) {
                std::unique_ptr<protocol::ClientMessage> requestMsg = protocol::codec::FlakeIdGeneratorNewIdBatchCodec::encodeRequest(
                        getName(), batchSize);
                auto invocation = spi::impl::ClientInvocation::create(getContext(), requestMsg,
                                                                      getName())->invoke();
                auto response =
                        protocol::codec::FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::decode(invocation.get());
                return flakeidgen::impl::IdBatch(response.base, response.increment, response.batchSize);
            }

            boost::future<bool> ClientFlakeIdGeneratorProxy::init(int64_t id) {
                // Add 1 hour worth of IDs as a reserve: due to long batch validity some clients might be still getting
                // older IDs. 1 hour is just a safe enough value, not a real guarantee: some clients might have longer
                // validity.
                // The init method should normally be called before any client generated IDs: in this case no reserve is
                // needed, so we don't want to increase the reserve excessively.
                int64_t reserve =
                        (int64_t) (3600 * 1000) /* 1 HOUR in milliseconds */ << (BITS_NODE_ID + BITS_SEQUENCE);
                return newId() >= id + reserve;
            }

            ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::FlakeIdBatchSupplier(ClientFlakeIdGeneratorProxy &proxy)
                    : proxy(proxy) {}

            flakeidgen::impl::IdBatch ClientFlakeIdGeneratorProxy::FlakeIdBatchSupplier::newIdBatch(int32_t batchSize) {
                return proxy.newIdBatch(batchSize);
            }

            IQueueImpl::IQueueImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:queueService", instanceName, context) {
                serialization::pimpl::Data data = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(data);
            }

            boost::future<std::string> IQueueImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            boost::future<bool> IQueueImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<bool> IQueueImpl::offer(const serialization::pimpl::Data &element, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::QueueOfferCodec::encodeRequest(getName(), element,
                                                                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool, protocol::codec::QueueOfferCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            boost::future<void> IQueueImpl::put(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::QueuePutCodec::encodeRequest(getName(), element);

                invokeOnPartition(request, partitionId);
            }

            boost::future<serialization::pimpl::Data> IQueueImpl::pollData(std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::QueuePollCodec::encodeRequest(getName(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::QueuePollCodec::ResponseParameters>(
                        request, partitionId);
            }

            int IQueueImpl::remainingCapacity() {
                auto request = protocol::codec::QueueRemainingCapacityCodec::encodeRequest(getName());

                return invokeAndGetFuture<int, protocol::codec::QueueRemainingCapacityCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> IQueueImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::QueueRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetFuture<bool, protocol::codec::QueueRemoveCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            boost::future<bool> IQueueImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::QueueContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetFuture<bool, protocol::codec::QueueContainsCodec::ResponseParameters>(request,
                                                                                                         partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drainToData(size_t maxElements) {
                auto request = protocol::codec::QueueDrainToMaxSizeCodec::encodeRequest(getName(), (int32_t) maxElements);

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::drainToData() {
                auto request = protocol::codec::QueueDrainToCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::QueueDrainToMaxSizeCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<serialization::pimpl::Data> IQueueImpl::peekData() {
                auto request = protocol::codec::QueuePeekCodec::encodeRequest(getName());

                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::QueuePeekCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<int> IQueueImpl::size() {
                auto request = protocol::codec::QueueSizeCodec::encodeRequest(getName());

                return invokeAndGetFuture<int, protocol::codec::QueueSizeCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            boost::future<bool> IQueueImpl::isEmpty() {
                auto request = protocol::codec::QueueIsEmptyCodec::encodeRequest(getName());

                return invokeAndGetFuture<bool, protocol::codec::QueueIsEmptyCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IQueueImpl::toArrayData() {
                auto request = protocol::codec::QueueIteratorCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::QueueIteratorCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> IQueueImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::QueueContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::QueueContainsAllCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            boost::future<bool> IQueueImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::QueueAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::QueueAddAllCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            boost::future<bool> IQueueImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::QueueCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::QueueCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> IQueueImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::QueueCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::QueueCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<void> IQueueImpl::clear() {
                auto request = protocol::codec::QueueClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            IQueueImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new QueueListenerMessageCodec(getName(), includeValue));
            }

            IQueueImpl::QueueListenerMessageCodec::QueueListenerMessageCodec(const std::string &name,
                                                                             bool includeValue) : name(name),
                                                                                                  includeValue(
                                                                                                          includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::QueueAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            boost::future<std::string> IQueueImpl::QueueListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::QueueAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IQueueImpl::QueueListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::QueueRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IQueueImpl::QueueListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::QueueRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            ProxyImpl::ProxyImpl(const std::string &serviceName, const std::string &objectName,
                                 spi::ClientContext *context)
                    : ClientProxy(objectName, serviceName, *context) {}

            ProxyImpl::~ProxyImpl() {}

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data &key) {
                return getContext().getPartitionService().getPartitionId(key);
            }

            boost::future<protocol::ClientMessage> ProxyImpl::invokeOnPartition(
                    std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
                try {
                    return spi::impl::ClientInvocation::create(getContext(), request, getName(), partitionId)->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                }
            }

            boost::future<protocol::ClientMessage> ProxyImpl::invoke(std::unique_ptr<protocol::ClientMessage> &request) {
                try {
                    return spi::impl::ClientInvocation::create(getContext(), request, getName())->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                }
            }

            boost::future<protocol::ClientMessage>
            ProxyImpl::invokeOnKeyOwner(std::unique_ptr<protocol::ClientMessage> &request,
                                        const serialization::pimpl::Data &keyData) {
                try {
                    return invokeOnPartition(request, getPartitionId(keyData));
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                }
            }

            boost::future<protocol::ClientMessage>
            ProxyImpl::invokeOnAddress(std::unique_ptr<protocol::ClientMessage> &request, const Address &address) {
                try {
                    auto invocation = spi::impl::ClientInvocation::create(getContext(), request, getName(), address);
                    return invocation->invoke();
                } catch (exception::IException &) {
                    util::ExceptionUtil::rethrow(std::current_exception());
                }
            }

            PartitionSpecificClientProxy::PartitionSpecificClientProxy(const std::string &serviceName,
                                                                       const std::string &objectName,
                                                                       spi::ClientContext *context) : ProxyImpl(
                    serviceName, objectName, context) {}

            void PartitionSpecificClientProxy::onInitialize() {
                std::string partitionKey = internal::partition::strategy::StringPartitioningStrategy::getPartitionKey(
                        name);
                partitionId = getContext().getPartitionService().getPartitionId(toData<std::string>(partitionKey));
            }
            
            IMapImpl::IMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instanceName, context) {
            }

            boost::future<bool> IMapImpl::containsKey(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapContainsKeyCodec::encodeRequest(getName(), key,
                                                                            util::getCurrentThreadId());
                return invokeAndGetFuture<bool, protocol::codec::MapContainsKeyCodec::ResponseParameters>(request,
                                                                                                          key);
            }

            boost::future<bool> IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MapContainsValueCodec::encodeRequest(getName(), value);
                return invokeAndGetFuture<bool, protocol::codec::MapContainsValueCodec::ResponseParameters>(request);
            }

            boost::future<serialization::pimpl::Data> IMapImpl::getData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapGetCodec::encodeRequest(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::MapGetCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<serialization::pimpl::Data> IMapImpl::removeData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapRemoveCodec::encodeRequest(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::MapRemoveCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<bool> IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MapRemoveIfSameCodec::encodeRequest(getName(), key, value,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<bool, protocol::codec::MapRemoveIfSameCodec::ResponseParameters>(request,
                                                                                                           key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::removeAll(const serialization::pimpl::Data &predicateData) {
                auto request = protocol::codec::MapRemoveAllCodec::encodeRequest(getName(), predicateData);
                return invoke(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapDeleteCodec::encodeRequest(getName(), key, util::getCurrentThreadId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::flush() {
                auto request = protocol::codec::MapFlushCodec::encodeRequest(getName());
                return invoke(request);
            }

            boost::future<bool> IMapImpl::tryRemove(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::MapTryRemoveCodec::encodeRequest(getName(), key,
                                                                          util::getCurrentThreadId(),
                                                                          std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool, protocol::codec::MapTryRemoveCodec::ResponseParameters>(request, key);
            }

            boost::future<bool> IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::MapTryPutCodec::encodeRequest(getName(), key, value,
                                                                       util::getCurrentThreadId(),
                                                                       std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool, protocol::codec::MapTryPutCodec::ResponseParameters>(request, key);
            }

            boost::future<serialization::pimpl::Data> IMapImpl::putData(const serialization::pimpl::Data &key,
                                                                          const serialization::pimpl::Data &value,
                                                                          std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::MapPutCodec::encodeRequest(getName(), key, value,
                                                                    util::getCurrentThreadId(),
                                                                    std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::MapPutCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::MapPutTransientCodec::encodeRequest(getName(), key, value,
                                                                             util::getCurrentThreadId(),
                                                                             std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<serialization::pimpl::Data> IMapImpl::putIfAbsentData(const serialization::pimpl::Data &key,
                                                                                  const serialization::pimpl::Data &value,
                                                                                  std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::MapPutIfAbsentCodec::encodeRequest(getName(), key, value,
                                                                            util::getCurrentThreadId(),
                                                                            std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count());
                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::MapPutIfAbsentCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<bool> IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                auto request = protocol::codec::MapReplaceIfSameCodec::encodeRequest(getName(), key, oldValue,
                                                                              newValue,
                                                                              util::getCurrentThreadId());

                return invokeAndGetFuture<bool, protocol::codec::MapReplaceIfSameCodec::ResponseParameters>(request,
                                                                                                            key);
            }

            boost::future<serialization::pimpl::Data> IMapImpl::replaceData(const serialization::pimpl::Data &key,
                                                                              const serialization::pimpl::Data &value) {
                auto request = protocol::codec::MapReplaceCodec::encodeRequest(getName(), key, value,
                                                                        util::getCurrentThreadId());

                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::MapReplaceCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<protocol::ClientMessage>
            IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                          std::chrono::steady_clock::duration ttl) {
                auto request = protocol::codec::MapSetCodec::encodeRequest(getName(), key, value,
                                                                           util::getCurrentThreadId(),
                                                                           std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                   ttl).count());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key) {
                return lock(key, -1);
            }

            boost::future<protocol::ClientMessage> IMapImpl::lock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration leaseTime) {
                auto request = protocol::codec::MapLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                     std::chrono::duration_cast<std::chrono::milliseconds>(leaseTime).count(),
                                                                     lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<bool> IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapIsLockedCodec::encodeRequest(getName(), key);

                return invokeAndGetFuture<bool, protocol::codec::MapIsLockedCodec::ResponseParameters>(request, key);
            }

            boost::future<bool> IMapImpl::tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::MapTryLockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(), -1,
                                                                        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
                                                                        lockReferenceIdGenerator->getNextReferenceId());
                return invokeAndGetFuture<bool, protocol::codec::MapTryLockCodec::ResponseParameters>(request, key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::unlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapUnlockCodec::encodeRequest(getName(), key, util::getCurrentThreadId(),
                                                                       lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<protocol::ClientMessage> IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapForceUnlockCodec::encodeRequest(getName(), key,
                                                                            lockReferenceIdGenerator->getNextReferenceId());
                return invokeOnPartition(request, getPartitionId(key));
            }

            boost::future<std::string> IMapImpl::addInterceptor(const serialization::pimpl::Data &interceptor) {
                auto request = protocol::codec::MapAddInterceptorCodec::encodeRequest(getName(), interceptor);
                return invokeAndGetFuture<std::string, protocol::codec::MapAddInterceptorCodec::ResponseParameters>(
                        request);
            }

            // TODO: We can use generic template Listener instead of impl::BaseEventHandler to prevent the virtual function calls
            boost::future<std::string> IMapImpl::addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                auto listenerFlags = EntryEvent::type::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags), std::move(entryEventHandler));
            }

            boost::future<std::string>
            IMapImpl::addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler,
                    Data &&predicate, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                EntryEvent::type listenerFlags = EntryEvent::type::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, std::move(predicate), listenerFlags), std::move(entryEventHandler));
            }

            boost::future<bool> IMapImpl::removeEntryListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<std::string> IMapImpl::addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler,
                                                   bool includeValue, Data &&key) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                EntryEvent::type listenerFlags = EntryEvent::type::ALL;
                return registerListener(createMapEntryListenerCodec(includeValue, listenerFlags, std::move(key)),
                                        std::move(entryEventHandler));
            }

            boost::future<std::unique_ptr<map::DataEntryView>> IMapImpl::getEntryViewData(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapGetEntryViewCodec::encodeRequest(getName(), key,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<std::unique_ptr<map::DataEntryView>, protocol::codec::MapGetEntryViewCodec::ResponseParameters>(
                        request, key);
            }

            boost::future<bool> IMapImpl::evict(const serialization::pimpl::Data &key) {
                auto request = protocol::codec::MapEvictCodec::encodeRequest(getName(), key, util::getCurrentThreadId());
                return invokeAndGetFuture<bool, protocol::codec::MapEvictCodec::ResponseParameters>(request,
                                                                                                    key);
            }

            boost::future<protocol::ClientMessage> IMapImpl::evictAll() {
                auto request = protocol::codec::MapEvictAllCodec::encodeRequest(getName());
                return invoke(request);
            }

            boost::future<EntryVector>
            IMapImpl::getAllData(int partitionId, std::vector<serialization::pimpl::Data> &&keys) {
                auto request = protocol::codec::MapGetAllCodec::encodeRequest(getName(), keys);
                return invokeAndGetFuture<EntryVector, protocol::codec::MapGetAllCodec>(request, partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::keySetData() {
                auto request = protocol::codec::MapKeySetCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetCodec::ResponseParameters>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::keySetData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapKeySetWithPredicateCodec::encodeRequest(getName(), predicate);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::keySetForPagingPredicateData(
                    const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapKeySetWithPagingPredicateCodec::encodeRequest(getName(), predicate);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MapKeySetWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<EntryVector> IMapImpl::entrySetData() {
                auto request = protocol::codec::MapEntrySetCodec::encodeRequest(getName());
                return invokeAndGetFuture<EntryVector, protocol::codec::MapEntrySetCodec::ResponseParameters>(request);
            }

            boost::future<EntryVector> IMapImpl::entrySetData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapEntriesWithPredicateCodec::encodeRequest(getName(), predicate);
                return invokeAndGetFuture<EntryVector, protocol::codec::MapEntriesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<EntryVector> IMapImpl::entrySetForPagingPredicateData(
                    const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapEntriesWithPagingPredicateCodec::encodeRequest(getName(), predicate);
                return invokeAndGetFuture<EntryVector, protocol::codec::MapEntriesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::valuesData() {
                auto request = protocol::codec::MapValuesCodec::encodeRequest(getName());
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesCodec::ResponseParameters>(
                        request);
            }

            boost::future<std::vector<serialization::pimpl::Data>> IMapImpl::valuesData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapValuesWithPredicateCodec::encodeRequest(getName(), predicate);
                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::MapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<EntryVector>
            IMapImpl::valuesForPagingPredicateData(const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapValuesWithPagingPredicateCodec::encodeRequest(predicate);
                return invokeAndGetFuture<EntryVector, protocol::codec::MapValuesWithPagingPredicateCodec::ResponseParameters>(
                        request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::addIndex(const std::string &attribute, bool ordered) {
                auto request = protocol::codec::MapAddIndexCodec::encodeRequest(getName(), attribute, ordered);
                return invoke(request);
            }

            boost::future<int> IMapImpl::size() {
                auto request = protocol::codec::MapSizeCodec::encodeRequest(getName());
                return invokeAndGetFuture<int, protocol::codec::MapSizeCodec::ResponseParameters>(request);
            }

            boost::future<bool> IMapImpl::isEmpty() {
                auto request = protocol::codec::MapIsEmptyCodec::encodeRequest(getName());
                return invokeAndGetFuture<bool, protocol::codec::MapIsEmptyCodec::ResponseParameters>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::putAllData(int partitionId, const EntryVector &entries) {
                auto request = protocol::codec::MapPutAllCodec::encodeRequest(getName(), entries);
                return invokeOnPartition(request, partitionId);
            }

            boost::future<protocol::ClientMessage> IMapImpl::clear() {
                auto request = protocol::codec::MapClearCodec::encodeRequest(getName());
                return invoke(request);
            }

            boost::future<serialization::pimpl::Data>
            IMapImpl::executeOnKeyData(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::MapExecuteOnKeyCodec::encodeRequest(getName(),
                                                                             processor,
                                                                             key,
                                                                             util::getCurrentThreadId());
                return invokeAndGetFuture<serialization::pimpl::Data,
                        protocol::codec::MapExecuteOnKeyCodec::ResponseParameters>(request, getPartitionId(key));
            }

            boost::future<serialization::pimpl::Data>
            IMapImpl::submitToKeyData(const serialization::pimpl::Data &key,
                                       const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::MapSubmitToKeyCodec::encodeRequest(getName(),
                                                                                   processor,
                                                                                   key,
                                                                                   util::getCurrentThreadId());
                return invokeAndGetFuture<serialization::pimpl::Data,
                        protocol::codec::MapExecuteOnKeyCodec::ResponseParameters>(request, getPartitionId(key));
            }

            boost::future<EntryVector> IMapImpl::executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                                    const serialization::pimpl::Data &processor) {
                auto request = protocol::codec::MapExecuteOnKeysCodec::encodeRequest(getName(), processor, keys);
                return invokeAndGetFuture<EntryVector,
                        protocol::codec::MapExecuteOnKeysCodec::ResponseParameters>(request);
            }

            boost::future<protocol::ClientMessage> IMapImpl::removeInterceptor(const std::string &id) {
                auto request = protocol::codec::MapRemoveInterceptorCodec::encodeRequest(getName(), id);
                return invoke(request);
            }

            boost::future<EntryVector> IMapImpl::executeOnEntriesData(const serialization::pimpl::Data &entryProcessor) {
                auto request = protocol::codec::MapExecuteOnAllKeysCodec::encodeRequest(
                        getName(), entryProcessor);
                return invokeAndGetFuture<EntryVector, protocol::codec::MapExecuteOnAllKeysCodec::ResponseParameters>(
                        request);

            }

            boost::future<EntryVector>
            IMapImpl::executeOnEntriesData(const serialization::pimpl::Data &entryProcessor, const serialization::pimpl::Data &predicate) {
                auto request = protocol::codec::MapExecuteWithPredicateCodec::encodeRequest(getName(),
                                                                                     entryProcessor,
                                                                                     predicate);
                return invokeAndGetFuture<EntryVector,
                        protocol::codec::MapExecuteWithPredicateCodec::ResponseParameters>(request);
            }


            std::unique_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&predicate,
                                                  EntryEvent::type listenerFlags) {
                return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerWithPredicateMessageCodec(getName(), includeValue, listenerFlags, std::move(predicate)));
            }

            std::unique_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, EntryEvent::type listenerFlags) {
                return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerMessageCodec(getName(), includeValue, listenerFlags));
            }

            std::unique_ptr<spi::impl::ListenerMessageCodec>
            IMapImpl::createMapEntryListenerCodec(bool includeValue, EntryEvent::type listenerFlags,
                                                  serialization::pimpl::Data &&key) {
                return std::unique_ptr<spi::impl::ListenerMessageCodec>(
                        new MapEntryListenerToKeyCodec(getName(), includeValue, listenerFlags, key));
            }

            void IMapImpl::onInitialize() {
                ProxyImpl::onInitialize();
                lockReferenceIdGenerator = getContext().getLockReferenceIdGenerator();
            }

            IMapImpl::MapEntryListenerMessageCodec::MapEntryListenerMessageCodec(const std::string &name,
                                                                                 bool includeValue,
                                                                                 EntryEvent::type listenerFlags) : name(name),
                                                                                                          includeValue(
                                                                                                                  includeValue),
                                                                                                          listenerFlags(
                                                                                                                  listenerFlags) {}

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerCodec::encodeRequest(name, includeValue,
                                                                                static_cast<int32_t>(listenerFlags),
                                                                                localOnly);
            }

            std::string IMapImpl::MapEntryListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec::encodeRequest(name, key, includeValue,
                                                                                     static_cast<int32_t>(listenerFlags), localOnly);
            }

            std::string IMapImpl::MapEntryListenerToKeyCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerToKeyCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerToKeyCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerToKeyCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            IMapImpl::MapEntryListenerToKeyCodec::MapEntryListenerToKeyCodec(const std::string &name, bool includeValue,
                                                                             EntryEvent::type listenerFlags,
                                                                             const serialization::pimpl::Data &key)
                    : name(name), includeValue(includeValue), listenerFlags(listenerFlags), key(key) {}

            IMapImpl::MapEntryListenerWithPredicateMessageCodec::MapEntryListenerWithPredicateMessageCodec(
                    const std::string &name, bool includeValue, EntryEvent::type listenerFlags,
                    serialization::pimpl::Data &&predicate) : name(name), includeValue(includeValue),
                                                             listenerFlags(listenerFlags), predicate(predicate) {}

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec::encodeRequest(name, predicate,
                                                                                             includeValue,
                                                                                             static_cast<int32_t>(listenerFlags), localOnly);
            }

            std::string IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::MapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            IMapImpl::MapEntryListenerWithPredicateMessageCodec::encodeRemoveRequest(
                    const std::string &realRegistrationId) const {
                return protocol::codec::MapRemoveEntryListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool IMapImpl::MapEntryListenerWithPredicateMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::MapRemoveEntryListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            TransactionalQueueImpl::TransactionalQueueImpl(const std::string &name,
                                                           txn::TransactionProxy *transactionProxy)
                    : TransactionalObject("hz:impl:queueService", name, transactionProxy) {

            }

            boost::future<bool> TransactionalQueueImpl::offer(const serialization::pimpl::Data &e, std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::TransactionalQueueOfferCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), e, std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<bool, protocol::codec::TransactionalQueueOfferCodec::ResponseParameters>(
                        request);
            }

            boost::future<serialization::pimpl::Data> TransactionalQueueImpl::pollData(std::chrono::steady_clock::duration timeout) {
                auto request = protocol::codec::TransactionalQueuePollCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId(), std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count());

                return invokeAndGetFuture<serialization::pimpl::Data, protocol::codec::TransactionalQueuePollCodec::ResponseParameters>(
                        request);
            }

            boost::future<int> TransactionalQueueImpl::size() {
                auto request = protocol::codec::TransactionalQueueSizeCodec::encodeRequest(
                                getName(), getTransactionId(), util::getCurrentThreadId());

                return invokeAndGetFuture<int, protocol::codec::TransactionalQueueSizeCodec::ResponseParameters>(
                        request);
            }

            ISetImpl::ISetImpl(const std::string &instanceName, spi::ClientContext *clientContext)
                    : ProxyImpl("hz:impl:setService", instanceName, clientContext) {
                serialization::pimpl::Data keyData = getContext().getSerializationService().toData<std::string>(
                        &instanceName);
                partitionId = getPartitionId(keyData);
            }

            boost::future<std::string> ISetImpl::addItemListener(impl::BaseEventHandler *itemEventHandler, bool includeValue) {
                return registerListener(createItemListenerCodec(includeValue), itemEventHandler);
            }

            boost::future<bool> ISetImpl::removeItemListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            boost::future<int> ISetImpl::size() {
                auto request = protocol::codec::SetSizeCodec::encodeRequest(
                        getName());

                return invokeAndGetFuture<int, protocol::codec::SetSizeCodec::ResponseParameters>(request, partitionId);
            }

            boost::future<bool> ISetImpl::isEmpty() {
                auto request = protocol::codec::SetIsEmptyCodec::encodeRequest(
                        getName());

                return invokeAndGetFuture<bool, protocol::codec::SetIsEmptyCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            boost::future<bool> ISetImpl::contains(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::SetContainsCodec::encodeRequest(getName(), element);

                return invokeAndGetFuture<bool, protocol::codec::SetContainsCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            boost::future<std::vector<serialization::pimpl::Data>> ISetImpl::toArrayData() {
                auto request = protocol::codec::SetGetAllCodec::encodeRequest(getName());

                return invokeAndGetFuture<std::vector<serialization::pimpl::Data>, protocol::codec::SetGetAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> ISetImpl::add(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::SetAddCodec::encodeRequest(getName(), element);

                return invokeAndGetFuture<bool, protocol::codec::SetAddCodec::ResponseParameters>(request, partitionId);
            }

            boost::future<bool> ISetImpl::remove(const serialization::pimpl::Data &element) {
                auto request = protocol::codec::SetRemoveCodec::encodeRequest(getName(), element);

                return invokeAndGetFuture<bool, protocol::codec::SetRemoveCodec::ResponseParameters>(request,
                                                                                                     partitionId);
            }

            boost::future<bool> ISetImpl::containsAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::SetContainsAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::SetContainsAllCodec::ResponseParameters>(request,
                                                                                                          partitionId);
            }

            boost::future<bool> ISetImpl::addAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::SetAddAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::SetAddAllCodec::ResponseParameters>(request,
                                                                                                     partitionId);
            }

            boost::future<bool> ISetImpl::removeAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::SetCompareAndRemoveAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::SetCompareAndRemoveAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<bool> ISetImpl::retainAll(const std::vector<serialization::pimpl::Data> &elements) {
                auto request = protocol::codec::SetCompareAndRetainAllCodec::encodeRequest(getName(), elements);

                return invokeAndGetFuture<bool, protocol::codec::SetCompareAndRetainAllCodec::ResponseParameters>(
                        request, partitionId);
            }

            boost::future<void> ISetImpl::clear() {
                auto request = protocol::codec::SetClearCodec::encodeRequest(getName());

                invokeOnPartition(request, partitionId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec>
            ISetImpl::createItemListenerCodec(bool includeValue) {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(
                        new SetListenerMessageCodec(getName(), includeValue));
            }

            ISetImpl::SetListenerMessageCodec::SetListenerMessageCodec(const std::string &name,
                                                                       bool includeValue) : name(name),
                                                                                            includeValue(
                                                                                                    includeValue) {}

            std::unique_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::SetAddListenerCodec::encodeRequest(name, includeValue, localOnly);
            }

            std::string ISetImpl::SetListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::SetAddListenerCodec::ResponseParameters::decode(responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            ISetImpl::SetListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::SetRemoveListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool ISetImpl::SetListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::SetRemoveListenerCodec::ResponseParameters::decode(clientMessage).response;
            }

            ITopicImpl::ITopicImpl(const std::string &instanceName, spi::ClientContext *context)
                    : proxy::ProxyImpl("hz:impl:topicService", instanceName, context) {
                partitionId = getPartitionId(toData(instanceName));
            }

            boost::future<void> ITopicImpl::publish(const serialization::pimpl::Data &data) {
                auto request = protocol::codec::TopicPublishCodec::encodeRequest(getName(), data);

                invokeOnPartition(request, partitionId);
            }

            boost::future<std::string> ITopicImpl::addMessageListener(impl::BaseEventHandler *topicEventHandler) {
                return registerListener(createItemListenerCodec(), topicEventHandler);
            }

            boost::future<bool> ITopicImpl::removeMessageListener(const std::string &registrationId) {
                return getContext().getClientListenerService().deregisterListener(registrationId);
            }

            std::shared_ptr<spi::impl::ListenerMessageCodec> ITopicImpl::createItemListenerCodec() {
                return std::shared_ptr<spi::impl::ListenerMessageCodec>(new TopicListenerMessageCodec(getName()));
            }

            ITopicImpl::TopicListenerMessageCodec::TopicListenerMessageCodec(const std::string &name) : name(name) {}

            std::unique_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeAddRequest(bool localOnly) const {
                return protocol::codec::TopicAddMessageListenerCodec::encodeRequest(name, localOnly);
            }

            std::string ITopicImpl::TopicListenerMessageCodec::decodeAddResponse(
                    protocol::ClientMessage &responseMessage) const {
                return protocol::codec::TopicAddMessageListenerCodec::ResponseParameters::decode(
                        responseMessage).response;
            }

            std::unique_ptr<protocol::ClientMessage>
            ITopicImpl::TopicListenerMessageCodec::encodeRemoveRequest(const std::string &realRegistrationId) const {
                return protocol::codec::TopicRemoveMessageListenerCodec::encodeRequest(name, realRegistrationId);
            }

            bool ITopicImpl::TopicListenerMessageCodec::decodeRemoveResponse(
                    protocol::ClientMessage &clientMessage) const {
                return protocol::codec::TopicRemoveMessageListenerCodec::ResponseParameters::decode(
                        clientMessage).response;
            }
        }

        namespace map {
            DataEntryView::DataEntryView(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                         int64_t cost,
                                         int64_t creationTime, int64_t expirationTime, int64_t hits,
                                         int64_t lastAccessTime,
                                         int64_t lastStoredTime, int64_t lastUpdateTime, int64_t version,
                                         int64_t evictionCriteriaNumber,
                                         int64_t ttl) : key(key), value(value), cost(cost), creationTime(creationTime),
                                                        expirationTime(expirationTime), hits(hits),
                                                        lastAccessTime(lastAccessTime),
                                                        lastStoredTime(lastStoredTime), lastUpdateTime(lastUpdateTime),
                                                        version(version),
                                                        evictionCriteriaNumber(evictionCriteriaNumber), ttl(ttl) {}


            const serialization::pimpl::Data &DataEntryView::getKey() const {
                return key;
            }

            const serialization::pimpl::Data &DataEntryView::getValue() const {
                return value;
            }

            int64_t DataEntryView::getCost() const {
                return cost;
            }

            int64_t DataEntryView::getCreationTime() const {
                return creationTime;
            }

            int64_t DataEntryView::getExpirationTime() const {
                return expirationTime;
            }

            int64_t DataEntryView::getHits() const {
                return hits;
            }

            int64_t DataEntryView::getLastAccessTime() const {
                return lastAccessTime;
            }

            int64_t DataEntryView::getLastStoredTime() const {
                return lastStoredTime;
            }

            int64_t DataEntryView::getLastUpdateTime() const {
                return lastUpdateTime;
            }

            int64_t DataEntryView::getVersion() const {
                return version;
            }

            int64_t DataEntryView::getEvictionCriteriaNumber() const {
                return evictionCriteriaNumber;
            }

            int64_t DataEntryView::getTtl() const {
                return ttl;
            }

        }

        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(Ringbuffer<ReliableTopicMessage> &rb,
                                                                 util::ILogger &logger)
                            : ringbuffer(rb), q(10), shutdown(false) {
                        runnerThread = std::thread([&]() { Task(ringbuffer, q, shutdown).run(); });
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {
                    }

                    void ReliableTopicExecutor::stop() {
                        bool expected = false;
                        if (!shutdown.compare_exchange_strong(expected, true)) {
                            return;
                        }

                        topic::impl::reliable::ReliableTopicExecutor::Message m;
                        m.type = topic::impl::reliable::ReliableTopicExecutor::CANCEL;
                        m.callback = NULL;
                        m.sequence = -1;
                        execute(m);
                        runnerThread.join();
                    }

                    void ReliableTopicExecutor::execute(const Message &m) {
                        q.push(m);
                    }

                    void ReliableTopicExecutor::Task::run() {
                        while (!shutdown) {
                            Message m = q.pop();
                            if (CANCEL == m.type) {
                                // exit the thread
                                return;
                            }
                            try {
                                proxy::ClientRingbufferProxy<ReliableTopicMessage> &ringbufferProxy =
                                        static_cast<proxy::ClientRingbufferProxy<ReliableTopicMessage> &>(rb);
                                auto future = ringbufferProxy.readManyAsync(m.sequence, 1, m.maxCount);
                                do {
                                    if (future.wait_for(boost::chrono::seconds(1)) == boost::future_status::ready) {
                                        std::shared_ptr<DataArray<ReliableTopicMessage> > allMessages(
                                                ringbufferProxy.getReadManyAsyncResponseObject(future.get()));

                                        m.callback->onResponse(allMessages);
                                        break;
                                    }
                                } while (!shutdown);
                            } catch (exception::IException &e) {
                                m.callback->onFailure(std::current_exception());
                            }
                        }
                    }

                    const std::string ReliableTopicExecutor::Task::getName() const {
                        return "ReliableTopicExecutor Task";
                    }

                    ReliableTopicExecutor::Task::Task(Ringbuffer<ReliableTopicMessage> &rb,
                                                      util::BlockingConcurrentQueue<ReliableTopicExecutor::Message> &q,
                                                      util::AtomicBoolean &shutdown) : rb(rb), q(q),
                                                                                       shutdown(shutdown) {}

                    ReliableTopicMessage::ReliableTopicMessage() {
                    }

                    ReliableTopicMessage::ReliableTopicMessage(
                            hazelcast::client::serialization::pimpl::Data payloadData,
                            std::unique_ptr<Address> &address)
                            : publishTime(util::currentTimeMillis()), publisherAddress(std::move(*address)),
                              payload(payloadData) {
                    }

                    int64_t ReliableTopicMessage::getPublishTime() const {
                        return publishTime;
                    }

                    const boost::optional<Address> &ReliableTopicMessage::getPublisherAddress() const {
                        return publisherAddress;
                    }

                    const serialization::pimpl::Data &ReliableTopicMessage::getPayload() const {
                        return payload;
                    }
                }
            }
        }

        namespace serialization {
            int32_t hz_serializer<topic::impl::reliable::ReliableTopicMessage>::getFactoryId() {
                return F_ID;
            }

            int hz_serializer<topic::impl::reliable::ReliableTopicMessage>::getClassId() {
                return RELIABLE_TOPIC_MESSAGE;
            }

            void hz_serializer<topic::impl::reliable::ReliableTopicMessage>::writeData(
                    const topic::impl::reliable::ReliableTopicMessage &object, ObjectDataOutput &out) {
                out.writeLong(object.publishTime);
                out.writeObject<Address>(object.publisherAddress.value_or(boost::none));
                out.writeData(&object.payload);
            }

            topic::impl::reliable::ReliableTopicMessage
            hz_serializer<topic::impl::reliable::ReliableTopicMessage>::readData(ObjectDataInput &in) {
                topic::impl::reliable::ReliableTopicMessage message;
                message.publishTime = in.readLong();
                message.publisherAddress = in.readObject<Address>();
                message.payload = in.readData();
                return message;
            }
        }

        MapEvent::MapEvent(const Member &member, EntryEvent::type eventType, const std::string &name,
                           int numberOfEntriesAffected)
                : member(member), eventType(eventType), name(name), numberOfEntriesAffected(numberOfEntriesAffected) {

        }

        Member MapEvent::getMember() const {
            return member;
        }

        EntryEvent::type MapEvent::getEventType() const {
            return eventType;
        }

        const std::string &MapEvent::getName() const {
            return name;
        }

        int MapEvent::getNumberOfEntriesAffected() const {
            return numberOfEntriesAffected;
        }

        std::ostream &operator<<(std::ostream &os, const MapEvent &event) {
            os << "MapEvent{member: " << event.member << " eventType: " << static_cast<int>(event.eventType) << " name: " << event.name
               << " numberOfEntriesAffected: " << event.numberOfEntriesAffected;
            return os;
        }

        ItemEventBase::ItemEventBase(const std::string &name, const Member &member, const ItemEventType &eventType)
                : name(name),
                  member(member),
                  eventType(
                          eventType) {}

        Member ItemEventBase::getMember() const {
            return member;
        }

        ItemEventType ItemEventBase::getEventType() const {
            return eventType;
        }

        std::string ItemEventBase::getName() const {
            return name;
        }

        ItemEventBase::~ItemEventBase() {
        }

        FlakeIdGenerator::FlakeIdGenerator(const std::shared_ptr<proxy::ClientFlakeIdGeneratorProxy> &impl) : impl_(impl) {}

        int64_t FlakeIdGenerator::newId() {
            return impl_->newId();
        }

        boost::future<bool> FlakeIdGenerator::init(int64_t id) {
            return impl_->init(id);
        }
    }
}
