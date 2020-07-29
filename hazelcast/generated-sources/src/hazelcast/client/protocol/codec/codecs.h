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
#pragma once

#include <string>
#include <vector>

#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

using namespace hazelcast::client;
using namespace hazelcast::util;
using namespace hazelcast::client::protocol;
using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_authentication_encode(const std::string &username, const std::string &password, const boost::optional<boost::uuids::uuid> &uuid, const boost::optional<boost::uuids::uuid> &ownerUuid, const bool &isOwnerConnection, const std::string &clientType, const byte &serializationVersion, const std::string &clientHazelcastVersion, const std::string &clientName, const std::vector<std::string> &labels, const int32_t &partitionCount, const boost::optional<boost::uuids::uuid> &clusterId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_authenticationcustom_encode(const Data &credentials, const boost::optional<boost::uuids::uuid> &uuid, const boost::optional<boost::uuids::uuid> &ownerUuid, const bool &isOwnerConnection, const std::string &clientType, const byte &serializationVersion, const std::string &clientHazelcastVersion, const std::string &clientName, const std::vector<std::string> &labels, const int32_t &partitionCount, const boost::optional<boost::uuids::uuid> &clusterId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_addmembershiplistener_encode(const bool &localOnly);

                struct HAZELCAST_API client_addmembershiplistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param member TODO DOC
                     * @param eventType TODO DOC
                    */
                    virtual void handle_member(const Member &member, const int32_t &eventType) = 0;
                    /**
                     * @param members TODO DOC
                    */
                    virtual void handle_memberlist(const std::vector<Member> &members) = 0;
                    /**
                     * @param member TODO DOC
                     * @param members TODO DOC
                     * @param key TODO DOC
                     * @param operationType TODO DOC
                     * @param value TODO DOC
                    */
                    virtual void handle_memberattributechange(const Member &member, const std::vector<Member> &members, const std::string &key, const int32_t &operationType, const boost::optional<std::string> &value) = 0;
                };

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_createproxy_encode(const std::string &name, const std::string &serviceName, const Address &target);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_destroyproxy_encode(const std::string &name, const std::string &serviceName);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_getpartitions_encode();

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_removealllisteners_encode();

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_addpartitionlostlistener_encode(const bool &localOnly);

                struct HAZELCAST_API client_addpartitionlostlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param partitionId TODO DOC
                     * @param lostBackupCount TODO DOC
                     * @param source TODO DOC
                    */
                    virtual void handle_partitionlost(const int32_t &partitionId, const int32_t &lostBackupCount, const boost::optional<Address> &source) = 0;
                };

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_removepartitionlostlistener_encode(const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_getdistributedobjects_encode();

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_adddistributedobjectlistener_encode(const bool &localOnly);

                struct HAZELCAST_API client_adddistributedobjectlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param name TODO DOC
                     * @param serviceName TODO DOC
                     * @param eventType TODO DOC
                    */
                    virtual void handle_distributedobject(const std::string &name, const std::string &serviceName, const std::string &eventType) = 0;
                };

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_removedistributedobjectlistener_encode(const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_ping_encode();

                /**
                 * The statistics is a String that is composed of key=value pairs separated by ',' . The following characters
                 * ('=' '.' ',' '\') should be escaped in IMap and ICache names by the escape character ('\'). E.g. if the map name is
                 * MyMap.First, it will be escaped as: MyMap\.First
                 * 
                 * The statistics key identify the category and name of the statistics. It is formatted as:
                 * mainCategory.subCategory.statisticName
                 * 
                 * An e.g. Operating system committedVirtualMemorySize path would be: os.committedVirtualMemorySize
                 * 
                 * Please note that if any client implementation can not provide the value for a statistics, the corresponding key, valaue
                 * pair will not be presented in the statistics string. Only the ones, that the client can provide will be added.
                 * 
                 * The statistics key names can be one of the following (Used IMap named <StatIMapName> and ICache Named
                 * <StatICacheName> and assuming that the near cache is configured):
                 * 
                 * clientType: The string that represents the client type. See {@link com.hazelcast.core.ClientType}
                 * 
                 * clusterConnectionTimestamp: The time that the client connected to the cluster (milliseconds since epoch). It is reset on
                 * each reconnection.
                 * 
                 * credentials.principal: The principal of the client if it exists. For
                 * {@link com.hazelcast.security.UsernamePasswordCredentials}, this is the username, for custom authentication it is set by
                 * the {@link com.hazelcast.security.Credentials} implementer.
                 * 
                 * clientAddress: The address of the client. It is formatted as "<IP>:<port>"
                 * 
                 * clientName: The name of the client instance. See ClientConfig.setInstanceName.
                 * 
                 * enterprise: "true" if the client is an enterprise client, "false" otherwise.
                 * 
                 * lastStatisticsCollectionTime: The time stamp (milliseconds since epoch) when the latest update for the statistics is
                 * collected.
                 * 
                 * Near cache statistics (see {@link com.hazelcast.monitor.NearCacheStats}):
                 * 
                 * nc.<StatIMapName>.creationTime: The creation time (milliseconds since epoch) of this Near Cache on the client.
                 * 
                 * nc.<StatIMapName>.evictions: The number of evictions of Near Cache entries owned by this client.
                 * 
                 * nc.<StatIMapName>.expirations: The number of TTL and max-idle expirations of Near Cache entries owned by the client.
                 * 
                 * nc.<StatIMapName>.hits: The number of hits (reads) of Near Cache entries owned by the client.
                 * 
                 * nc.<StatIMapName>.lastPersistenceDuration: The duration in milliseconds of the last Near Cache key persistence
                 * (when the pre-load feature is enabled).
                 * 
                 * nc.<StatIMapName>.lastPersistenceFailure: The failure reason of the last Near Cache persistence (when the pre-load
                 * feature is enabled).
                 * 
                 * nc.<StatIMapName>.lastPersistenceKeyCount: The number of Near Cache key persistences (when the pre-load feature is
                 * enabled).
                 * 
                 * nc.<StatIMapName>.lastPersistenceTime: The timestamp (milliseconds since epoch) of the last Near Cache key
                 * persistence (when the pre-load feature is enabled).
                 * 
                 * nc.<StatIMapName>.lastPersistenceWrittenBytes: The written number of bytes of the last Near Cache key persistence
                 * (when the pre-load feature is enabled).
                 * 
                 * nc.<StatIMapName>.misses: The number of misses of Near Cache entries owned by the client.
                 * 
                 * nc.<StatIMapName>.ownedEntryCount: the number of Near Cache entries owned by the client.
                 * 
                 * nc.<StatIMapName>.ownedEntryMemoryCost: Memory cost (number of bytes) of Near Cache entries owned by the client.
                 * 
                 * nc.hz/<StatICacheName>.creationTime: The creation time of this Near Cache on the client.
                 * 
                 * nc.hz/<StatICacheName>.evictions: The number of evictions of Near Cache entries owned by the client.
                 * 
                 * nc.hz/<StatICacheName>.expirations: The number of TTL and max-idle expirations of Near Cache entries owned by the
                 * client.
                 * 
                 * nc.hz/<StatICacheName>.hits
                 * nc.hz/<StatICacheName>.lastPersistenceDuration
                 * nc.hz/<StatICacheName>.lastPersistenceFailure
                 * nc.hz/<StatICacheName>.lastPersistenceKeyCount
                 * nc.hz/<StatICacheName>.lastPersistenceTime
                 * nc.hz/<StatICacheName>.lastPersistenceWrittenBytes
                 * nc.hz/<StatICacheName>.misses
                 * nc.hz/<StatICacheName>.ownedEntryCount
                 * nc.hz/<StatICacheName>.ownedEntryMemoryCost
                 * 
                 * Operating System Statistics (see {@link com.hazelcast.internal.metrics.metricsets.OperatingSystemMetricSet},
                 * {@link sun.management.OperatingSystemImpl}) and {@link com.sun.management.UnixOperatingSystemMXBean}:
                 * 
                 * os.committedVirtualMemorySize: The amount of virtual memory that is guaranteed to be available to the running process in
                 * bytes, or -1 if this operation is not supported.
                 * 
                 * os.freePhysicalMemorySize: The amount of free physical memory in bytes.
                 * 
                 * os.freeSwapSpaceSize: The amount of free swap space in bytes.
                 * 
                 * os.maxFileDescriptorCount: The maximum number of file descriptors.
                 * 
                 * os.openFileDescriptorCount: The number of open file descriptors.
                 * 
                 * os.processCpuTime: The CPU time used by the process in nanoseconds.
                 * 
                 * os.systemLoadAverage: The system load average for the last minute. (See
                 * {@link java.lang.management.OperatingSystemMXBean#getSystemLoadAverage})
                 * The system load average is the sum of the number of runnable entities
                 * queued to the {@link java.lang.management.OperatingSystemMXBean#getAvailableProcessors} available processors
                 * and the number of runnable entities running on the available processors
                 * averaged over a period of time.
                 * The way in which the load average is calculated is operating system
                 * specific but is typically a damped time-dependent average.
                 * <p>
                 * If the load average is not available, a negative value is returned.
                 * <p>
                 * 
                 * os.totalPhysicalMemorySize: The total amount of physical memory in bytes.
                 * 
                 * os.totalSwapSpaceSize: The total amount of swap space in bytes.
                 * 
                 * Runtime statistics (See {@link Runtime}:
                 * 
                 * runtime.availableProcessors: The number of processors available to the process.
                 * 
                 * runtime.freeMemory: an approximation to the total amount of memory currently available for future allocated objects,
                 * measured in bytes.
                 * 
                 * runtime.maxMemory: The maximum amount of memory that the process will  attempt to use, measured in bytes
                 * 
                 * runtime.totalMemory: The total amount of memory currently available for current and future objects, measured in bytes.
                 * 
                 * runtime.uptime: The uptime of the process in milliseconds.
                 * 
                 * runtime.usedMemory: The difference of total memory and used memory in bytes.
                 * 
                 * userExecutor.queueSize: The number of waiting tasks in the client user executor (See ClientExecutionService#getUserExecutor)
                 * 
                 * Not: Please observe that the name for the ICache appears to be the hazelcast instance name "hz" followed by "/" and
                 * followed by the cache name provided which is StatICacheName.
                 * 
                 * An example stats string (IMap name: StatIMapName and ICache name: StatICacheName with near-cache enabled):
                 * 
                 * lastStatisticsCollectionTime=1496137027173,enterprise=false,clientType=JAVA,clusterConnectionTimestamp=1496137018114,
                 * clientAddress=127.0.0.1:5001,clientName=hz.client_0,executionService.userExecutorQueueSize=0,runtime.maxMemory=1065025536,
                 * os.freePhysicalMemorySize=32067584,os.totalPhysicalMemorySize=17179869184,os.systemLoadAverage=249,
                 * runtime.usedMemory=16235040,runtime.freeMemory=115820000,os.totalSwapSpaceSize=5368709120,runtime.availableProcessors=4,
                 * runtime.uptime=13616,os.committedVirtualMemorySize=4081422336,os.maxFileDescriptorCount=10240,
                 * runtime.totalMemory=132055040,os.processCpuTime=6270000000,os.openFileDescriptorCount=67,os.freeSwapSpaceSize=888406016,
                 * nc.StatIMapName.creationTime=1496137021761,nc.StatIMapName.evictions=0,nc.StatIMapName.hits=1,
                 * nc.StatIMapName.lastPersistenceDuration=0,nc.StatIMapName.lastPersistenceKeyCount=0,nc.StatIMapName.lastPersistenceTime=0,
                 * nc.StatIMapName.lastPersistenceWrittenBytes=0,nc.StatIMapName.misses=1,nc.StatIMapName.ownedEntryCount=1,
                 * nc.StatIMapName.expirations=0,nc.StatIMapName.ownedEntryMemoryCost=140,nc.hz/StatICacheName.creationTime=1496137025201,
                 * nc.hz/StatICacheName.evictions=0,nc.hz/StatICacheName.hits=1,nc.hz/StatICacheName.lastPersistenceDuration=0,
                 * nc.hz/StatICacheName.lastPersistenceKeyCount=0,nc.hz/StatICacheName.lastPersistenceTime=0,
                 * nc.hz/StatICacheName.lastPersistenceWrittenBytes=0,nc.hz/StatICacheName.misses=1,nc.hz/StatICacheName.ownedEntryCount=1,
                 * nc.hz/StatICacheName.expirations=0,nc.hz/StatICacheName.ownedEntryMemoryCost=140
                 */
                 ClientMessage HAZELCAST_API client_statistics_encode(const std::string &stats);

                /**
                 * Deploys the list of classes to cluster
                 * Each item is a Map.Entry<String, byte[]> in the list.
                 * key of entry is full class name, and byte[] is the class definition.
                 */
                 ClientMessage HAZELCAST_API client_deployclasses_encode(const std::vector<std::pair<std::string, std::vector<byte>>> &classDefinitions);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_addpartitionlistener_encode();

                struct HAZELCAST_API client_addpartitionlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param partitions TODO DOC
                     * @param partitionStateVersion TODO DOC
                    */
                    virtual void handle_partitions(const std::vector<std::pair<Address, std::vector<int32_t>>> &partitions, const int32_t &partitionStateVersion) = 0;
                };

                /**
                 * Proxies will be created on all cluster members.
                 * If the member is  a lite member, a replicated map will not be created.
                 * Any proxy creation failure is logged on the server side.
                 * Exceptions related to a proxy creation failure is not send to the client.
                 * A proxy creation failure does not cancel this operation, all proxies will be attempted to be created.
                 */
                 ClientMessage HAZELCAST_API client_createproxies_encode(const std::vector<std::pair<std::string, std::string>> &proxies);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API client_isfailoversupported_encode();

                /**
                 * Adds listener for backup acks
                 */
                 ClientMessage HAZELCAST_API client_localbackuplistener_encode();

                struct HAZELCAST_API client_localbackuplistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param sourceInvocationCorrelationId correlation id of the invocation that backup acks belong to
                    */
                    virtual void handle_backup(const int64_t &sourceInvocationCorrelationId) = 0;
                };

                /**
                 * Publishes the message to all subscribers of this topic
                 */
                 ClientMessage HAZELCAST_API topic_publish_encode(const std::string &name, const Data &message);

                /**
                 * Subscribes to this topic. When someone publishes a message on this topic. onMessage() function of the given
                 * MessageListener is called. More than one message listener can be added on one instance.
                 */
                 ClientMessage HAZELCAST_API topic_addmessagelistener_encode(const std::string &name, const bool &localOnly);

                struct HAZELCAST_API topic_addmessagelistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param item TODO DOC
                     * @param publishTime TODO DOC
                     * @param uuid TODO DOC
                    */
                    virtual void handle_topic(const Data &item, const int64_t &publishTime, const boost::optional<boost::uuids::uuid> &uuid) = 0;
                };

                /**
                 * Stops receiving messages for the given message listener.If the given listener already removed, this method does nothing.
                 */
                 ClientMessage HAZELCAST_API topic_removemessagelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Associates a given value to the specified key and replicates it to the cluster. If there is an old value, it will
                 * be replaced by the specified one and returned from the call. In addition, you have to specify a ttl and its TimeUnit
                 * to define when the value is outdated and thus should be removed from the replicated map.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_put_encode(const std::string &name, const Data &key, const Data &value, const int64_t &ttl);

                /**
                 * Returns the number of key-value mappings in this map. If the map contains more than Integer.MAX_VALUE elements,
                 * returns Integer.MAX_VALUE.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_size_encode(const std::string &name);

                /**
                 * Return true if this map contains no key-value mappings
                 */
                 ClientMessage HAZELCAST_API replicatedmap_isempty_encode(const std::string &name);

                /**
                 * Returns true if this map contains a mapping for the specified key.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_containskey_encode(const std::string &name, const Data &key);

                /**
                 * Returns true if this map maps one or more keys to the specified value.
                 * This operation will probably require time linear in the map size for most implementations of the Map interface.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_containsvalue_encode(const std::string &name, const Data &value);

                /**
                 * Returns the value to which the specified key is mapped, or null if this map contains no mapping for the key.
                 * If this map permits null values, then a return value of null does not
                 * necessarily indicate that the map contains no mapping for the key; it's also possible that the map
                 * explicitly maps the key to null.  The #containsKey operation may be used to distinguish these two cases.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_get_encode(const std::string &name, const Data &key);

                /**
                 * Removes the mapping for a key from this map if it is present (optional operation). Returns the value to which this map previously associated the key,
                 * or null if the map contained no mapping for the key. If this map permits null values, then a return value of
                 * null does not necessarily indicate that the map contained no mapping for the key; it's also possible that the map
                 * explicitly mapped the key to null. The map will not contain a mapping for the specified key once the call returns.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_remove_encode(const std::string &name, const Data &key);

                /**
                 * Copies all of the mappings from the specified map to this map (optional operation). The effect of this call is
                 * equivalent to that of calling put(Object,Object) put(k, v) on this map once for each mapping from key k to value
                 * v in the specified map. The behavior of this operation is undefined if the specified map is modified while the
                 * operation is in progress.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_putall_encode(const std::string &name, const std::vector<std::pair<Data, Data>> &entries);

                /**
                 * The clear operation wipes data out of the replicated maps.It is the only synchronous remote operation in this
                 * implementation, so be aware that this might be a slow operation. If some node fails on executing the operation,
                 * it is retried for at most 3 times (on the failing nodes only). If it does not work after the third time, this
                 * method throws a OPERATION_TIMEOUT back to the caller.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_clear_encode(const std::string &name);

                /**
                 * Adds an continuous entry listener for this map. The listener will be notified for map add/remove/update/evict
                 * events filtered by the given predicate.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_addentrylistenertokeywithpredicate_encode(const std::string &name, const Data &key, const Data &predicate, const bool &localOnly);

                struct HAZELCAST_API replicatedmap_addentrylistenertokeywithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key Key with which the specified value is to be associated.
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds an continuous entry listener for this map. The listener will be notified for map add/remove/update/evict
                 * events filtered by the given predicate.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_addentrylistenerwithpredicate_encode(const std::string &name, const Data &predicate, const bool &localOnly);

                struct HAZELCAST_API replicatedmap_addentrylistenerwithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds the specified entry listener for the specified key. The listener will be notified for all
                 * add/remove/update/evict events of the specified key only.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_addentrylistenertokey_encode(const std::string &name, const Data &key, const bool &localOnly);

                struct HAZELCAST_API replicatedmap_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key Key with which the specified value is to be associated.
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds an entry listener for this map. The listener will be notified for all map add/remove/update/evict events.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_addentrylistener_encode(const std::string &name, const bool &localOnly);

                struct HAZELCAST_API replicatedmap_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Removes the specified entry listener. Returns silently if there was no such listener added before.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Returns a lazy Set view of the key contained in this map. A LazySet is optimized for querying speed
                 * (preventing eager deserialization and hashing on HashSet insertion) and does NOT provide all operations.
                 * Any kind of mutating function will throw an UNSUPPORTED_OPERATION. Same is true for operations
                 * like java.util.Set#contains(Object) and java.util.Set#containsAll(java.util.Collection) which would result in
                 * very poor performance if called repeatedly (for example, in a loop). If the use case is different from querying
                 * the data, please copy the resulting set into a new java.util.HashSet.
                 */
                 ClientMessage HAZELCAST_API replicatedmap_keyset_encode(const std::string &name);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API replicatedmap_values_encode(const std::string &name);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API replicatedmap_entryset_encode(const std::string &name);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API replicatedmap_addnearcacheentrylistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API replicatedmap_addnearcacheentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Query operation to retrieve the current value of the PNCounter.
                 * <p>
                 * The invocation will return the replica timestamps (vector clock) which
                 * can then be sent with the next invocation to keep session consistency
                 * guarantees.
                 * The target replica is determined by the {@code targetReplica} parameter.
                 * If smart routing is disabled, the actual member processing the client
                 * message may act as a proxy.
                 */
                 ClientMessage HAZELCAST_API pncounter_get_encode(const std::string &name, const std::vector<std::pair<boost::uuids::uuid, int64_t>> &replicaTimestamps, const Address &targetReplica);

                /**
                 * Adds a delta to the PNCounter value. The delta may be negative for a
                 * subtraction.
                 * <p>
                 * The invocation will return the replica timestamps (vector clock) which
                 * can then be sent with the next invocation to keep session consistency
                 * guarantees.
                 * The target replica is determined by the {@code targetReplica} parameter.
                 * If smart routing is disabled, the actual member processing the client
                 * message may act as a proxy.
                 */
                 ClientMessage HAZELCAST_API pncounter_add_encode(const std::string &name, const int64_t &delta, const bool &getBeforeUpdate, const std::vector<std::pair<boost::uuids::uuid, int64_t>> &replicaTimestamps, const Address &targetReplica);

                /**
                 * Returns the configured number of CRDT replicas for the PN counter with
                 * the given {@code name}.
                 * The actual replica count may be less, depending on the number of data
                 * members in the cluster (members that own data).
                 */
                 ClientMessage HAZELCAST_API pncounter_getconfiguredreplicacount_encode(const std::string &name);

                /**
                 * Returns the number of elements in this set (its cardinality). If this set contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE.
                 */
                 ClientMessage HAZELCAST_API set_size_encode(const std::string &name);

                /**
                 * Returns true if this set contains the specified element.
                 */
                 ClientMessage HAZELCAST_API set_contains_encode(const std::string &name, const Data &value);

                /**
                 * Returns true if this set contains all of the elements of the specified collection. If the specified collection is
                 * also a set, this method returns true if it is a subset of this set.
                 */
                 ClientMessage HAZELCAST_API set_containsall_encode(const std::string &name, const std::vector<Data> &items);

                /**
                 * Adds the specified element to this set if it is not already present (optional operation).
                 * If this set already contains the element, the call leaves the set unchanged and returns false.In combination with
                 * the restriction on constructors, this ensures that sets never contain duplicate elements.
                 * The stipulation above does not imply that sets must accept all elements; sets may refuse to add any particular
                 * element, including null, and throw an exception, as described in the specification for Collection
                 * Individual set implementations should clearly document any restrictions on the elements that they may contain.
                 */
                 ClientMessage HAZELCAST_API set_add_encode(const std::string &name, const Data &value);

                /**
                 * Removes the specified element from this set if it is present (optional operation).
                 * Returns true if this set contained the element (or equivalently, if this set changed as a result of the call).
                 * (This set will not contain the element once the call returns.)
                 */
                 ClientMessage HAZELCAST_API set_remove_encode(const std::string &name, const Data &value);

                /**
                 * Adds all of the elements in the specified collection to this set if they're not already present
                 * (optional operation). If the specified collection is also a set, the addAll operation effectively modifies this
                 * set so that its value is the union of the two sets. The behavior of this operation is undefined if the specified
                 * collection is modified while the operation is in progress.
                 */
                 ClientMessage HAZELCAST_API set_addall_encode(const std::string &name, const std::vector<Data> &valueList);

                /**
                 * Removes from this set all of its elements that are contained in the specified collection (optional operation).
                 * If the specified collection is also a set, this operation effectively modifies this set so that its value is the
                 * asymmetric set difference of the two sets.
                 */
                 ClientMessage HAZELCAST_API set_compareandremoveall_encode(const std::string &name, const std::vector<Data> &values);

                /**
                 * Retains only the elements in this set that are contained in the specified collection (optional operation).
                 * In other words, removes from this set all of its elements that are not contained in the specified collection.
                 * If the specified collection is also a set, this operation effectively modifies this set so that its value is the
                 * intersection of the two sets.
                 */
                 ClientMessage HAZELCAST_API set_compareandretainall_encode(const std::string &name, const std::vector<Data> &values);

                /**
                 * Removes all of the elements from this set (optional operation). The set will be empty after this call returns.
                 */
                 ClientMessage HAZELCAST_API set_clear_encode(const std::string &name);

                /**
                 * Return the all elements of this collection
                 */
                 ClientMessage HAZELCAST_API set_getall_encode(const std::string &name);

                /**
                 * Adds an item listener for this collection. Listener will be notified for all collection add/remove events.
                 */
                 ClientMessage HAZELCAST_API set_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API set_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param item TODO DOC
                     * @param uuid TODO DOC
                     * @param eventType TODO DOC
                    */
                    virtual void handle_item(const boost::optional<Data> &item, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &eventType) = 0;
                };

                /**
                 * Removes the specified item listener. Returns silently if the specified listener was not added before.
                 */
                 ClientMessage HAZELCAST_API set_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Returns true if this set contains no elements.
                 */
                 ClientMessage HAZELCAST_API set_isempty_encode(const std::string &name);

                /**
                 * Initiates an orderly shutdown in which previously submitted tasks are executed, but no new tasks will be accepted.
                 * Invocation has no additional effect if already shut down.
                 */
                 ClientMessage HAZELCAST_API executorservice_shutdown_encode(const std::string &name);

                /**
                 * Returns true if this executor has been shut down.
                 */
                 ClientMessage HAZELCAST_API executorservice_isshutdown_encode(const std::string &name);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API executorservice_cancelonpartition_encode(const boost::optional<boost::uuids::uuid> &uuid, const bool &interrupt);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API executorservice_cancelonaddress_encode(const boost::optional<boost::uuids::uuid> &uuid, const Address &address, const bool &interrupt);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API executorservice_submittopartition_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &uuid, const Data &callable);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API executorservice_submittoaddress_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &uuid, const Data &callable, const Address &address);

                /**
                 * Add new item to transactional set.
                 */
                 ClientMessage HAZELCAST_API transactionalset_add_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item);

                /**
                 * Remove item from transactional set.
                 */
                 ClientMessage HAZELCAST_API transactionalset_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item);

                /**
                 * Returns the size of the set.
                 */
                 ClientMessage HAZELCAST_API transactionalset_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Returns the number of elements in this list.  If this list contains more than Integer.MAX_VALUE elements, returns
                 * Integer.MAX_VALUE.
                 */
                 ClientMessage HAZELCAST_API list_size_encode(const std::string &name);

                /**
                 * Returns true if this list contains the specified element.
                 */
                 ClientMessage HAZELCAST_API list_contains_encode(const std::string &name, const Data &value);

                /**
                 * Returns true if this list contains all of the elements of the specified collection.
                 */
                 ClientMessage HAZELCAST_API list_containsall_encode(const std::string &name, const std::vector<Data> &values);

                /**
                 * Appends the specified element to the end of this list (optional operation). Lists that support this operation may
                 * place limitations on what elements may be added to this list.  In particular, some lists will refuse to add null
                 * elements, and others will impose restrictions on the type of elements that may be added. List classes should
                 * clearly specify in their documentation any restrictions on what elements may be added.
                 */
                 ClientMessage HAZELCAST_API list_add_encode(const std::string &name, const Data &value);

                /**
                 * Removes the first occurrence of the specified element from this list, if it is present (optional operation).
                 * If this list does not contain the element, it is unchanged.
                 * Returns true if this list contained the specified element (or equivalently, if this list changed as a result of the call).
                 */
                 ClientMessage HAZELCAST_API list_remove_encode(const std::string &name, const Data &value);

                /**
                 * Appends all of the elements in the specified collection to the end of this list, in the order that they are
                 * returned by the specified collection's iterator (optional operation).
                 * The behavior of this operation is undefined if the specified collection is modified while the operation is in progress.
                 * (Note that this will occur if the specified collection is this list, and it's nonempty.)
                 */
                 ClientMessage HAZELCAST_API list_addall_encode(const std::string &name, const std::vector<Data> &valueList);

                /**
                 * Removes from this list all of its elements that are contained in the specified collection (optional operation).
                 */
                 ClientMessage HAZELCAST_API list_compareandremoveall_encode(const std::string &name, const std::vector<Data> &values);

                /**
                 * Retains only the elements in this list that are contained in the specified collection (optional operation).
                 * In other words, removes from this list all of its elements that are not contained in the specified collection.
                 */
                 ClientMessage HAZELCAST_API list_compareandretainall_encode(const std::string &name, const std::vector<Data> &values);

                /**
                 * Removes all of the elements from this list (optional operation). The list will be empty after this call returns.
                 */
                 ClientMessage HAZELCAST_API list_clear_encode(const std::string &name);

                /**
                 * Return the all elements of this collection
                 */
                 ClientMessage HAZELCAST_API list_getall_encode(const std::string &name);

                /**
                 * Adds an item listener for this collection. Listener will be notified for all collection add/remove events.
                 */
                 ClientMessage HAZELCAST_API list_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API list_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param item TODO DOC
                     * @param uuid TODO DOC
                     * @param eventType TODO DOC
                    */
                    virtual void handle_item(const boost::optional<Data> &item, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &eventType) = 0;
                };

                /**
                 * Removes the specified item listener. Returns silently if the specified listener was not added before.
                 */
                 ClientMessage HAZELCAST_API list_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Returns true if this list contains no elements
                 */
                 ClientMessage HAZELCAST_API list_isempty_encode(const std::string &name);

                /**
                 * Inserts all of the elements in the specified collection into this list at the specified position (optional operation).
                 * Shifts the element currently at that position (if any) and any subsequent elements to the right (increases their indices).
                 * The new elements will appear in this list in the order that they are returned by the specified collection's iterator.
                 * The behavior of this operation is undefined if the specified collection is modified while the operation is in progress.
                 * (Note that this will occur if the specified collection is this list, and it's nonempty.)
                 */
                 ClientMessage HAZELCAST_API list_addallwithindex_encode(const std::string &name, const int32_t &index, const std::vector<Data> &valueList);

                /**
                 * Returns the element at the specified position in this list
                 */
                 ClientMessage HAZELCAST_API list_get_encode(const std::string &name, const int32_t &index);

                /**
                 * The element previously at the specified position
                 */
                 ClientMessage HAZELCAST_API list_set_encode(const std::string &name, const int32_t &index, const Data &value);

                /**
                 * Inserts the specified element at the specified position in this list (optional operation). Shifts the element
                 * currently at that position (if any) and any subsequent elements to the right (adds one to their indices).
                 */
                 ClientMessage HAZELCAST_API list_addwithindex_encode(const std::string &name, const int32_t &index, const Data &value);

                /**
                 * Removes the element at the specified position in this list (optional operation). Shifts any subsequent elements
                 * to the left (subtracts one from their indices). Returns the element that was removed from the list.
                 */
                 ClientMessage HAZELCAST_API list_removewithindex_encode(const std::string &name, const int32_t &index);

                /**
                 * Returns the index of the last occurrence of the specified element in this list, or -1 if this list does not
                 * contain the element.
                 */
                 ClientMessage HAZELCAST_API list_lastindexof_encode(const std::string &name, const Data &value);

                /**
                 * Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not
                 * contain the element.
                 */
                 ClientMessage HAZELCAST_API list_indexof_encode(const std::string &name, const Data &value);

                /**
                 * Returns a view of the portion of this list between the specified from, inclusive, and to, exclusive.(If from and
                 * to are equal, the returned list is empty.) The returned list is backed by this list, so non-structural changes in
                 * the returned list are reflected in this list, and vice-versa. The returned list supports all of the optional list
                 * operations supported by this list.
                 * This method eliminates the need for explicit range operations (of the sort that commonly exist for arrays).
                 * Any operation that expects a list can be used as a range operation by passing a subList view instead of a whole list.
                 * Similar idioms may be constructed for indexOf and lastIndexOf, and all of the algorithms in the Collections class
                 * can be applied to a subList.
                 * The semantics of the list returned by this method become undefined if the backing list (i.e., this list) is
                 * structurally modified in any way other than via the returned list.(Structural modifications are those that change
                 * the size of this list, or otherwise perturb it in such a fashion that iterations in progress may yield incorrect results.)
                 */
                 ClientMessage HAZELCAST_API list_sub_encode(const std::string &name, const int32_t &from, const int32_t &to);

                /**
                 * Returns an iterator over the elements in this list in proper sequence.
                 */
                 ClientMessage HAZELCAST_API list_iterator_encode(const std::string &name);

                /**
                 * Returns a list iterator over the elements in this list (in proper sequence), starting at the specified position
                 * in the list. The specified index indicates the first element that would be returned by an initial call to
                 * ListIterator#next next. An initial call to ListIterator#previous previous would return the element with the
                 * specified index minus one.
                 */
                 ClientMessage HAZELCAST_API list_listiterator_encode(const std::string &name, const int32_t &index);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API transaction_commit_encode(const boost::optional<boost::uuids::uuid> &transactionId, const int64_t &threadId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API transaction_create_encode(const int64_t &timeout, const int32_t &durability, const int32_t &transactionType, const int64_t &threadId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API transaction_rollback_encode(const boost::optional<boost::uuids::uuid> &transactionId, const int64_t &threadId);

                /**
                 * Inserts the specified element into this queue, waiting up to the specified wait time if necessary for space to
                 * become available.
                 */
                 ClientMessage HAZELCAST_API transactionalqueue_offer_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item, const int64_t &timeout);

                /**
                 * Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
                 */
                 ClientMessage HAZELCAST_API transactionalqueue_take_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element
                 * to become available.
                 */
                 ClientMessage HAZELCAST_API transactionalqueue_poll_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const int64_t &timeout);

                /**
                 * Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
                 */
                 ClientMessage HAZELCAST_API transactionalqueue_peek_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const int64_t &timeout);

                /**
                 * Returns the number of elements in this collection.If this collection contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE.
                 */
                 ClientMessage HAZELCAST_API transactionalqueue_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Stores a key-value pair in the multimap.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_put_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * Returns the collection of values associated with the key.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_get_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Removes the given key value pair from the multimap.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Removes all the entries associated with the given key.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_removeentry_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * Returns the number of values matching the given key in the multimap.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_valuecount_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Returns the number of key-value pairs in the multimap.
                 */
                 ClientMessage HAZELCAST_API transactionalmultimap_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Reads the recorded metrics starting with the smallest sequence number
                 * greater or equals to the sequence number set in fromSequence.
                 */
                 ClientMessage HAZELCAST_API metrics_readmetrics_encode(const boost::optional<boost::uuids::uuid> &uuid, const int64_t &fromSequence);

                /**
                 * Returns number of items in the ringbuffer. If no ttl is set, the size will always be equal to capacity after the
                 * head completed the first looparound the ring. This is because no items are getting retired.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_size_encode(const std::string &name);

                /**
                 * Returns the sequence of the tail. The tail is the side of the ringbuffer where the items are added to.
                 * The initial value of the tail is -1.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_tailsequence_encode(const std::string &name);

                /**
                 * Returns the sequence of the head. The head is the side of the ringbuffer where the oldest items in the ringbuffer
                 * are found. If the RingBuffer is empty, the head will be one more than the tail.
                 * The initial value of the head is 0 (1 more than tail).
                 */
                 ClientMessage HAZELCAST_API ringbuffer_headsequence_encode(const std::string &name);

                /**
                 * Returns the capacity of this Ringbuffer.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_capacity_encode(const std::string &name);

                /**
                 * Returns the remaining capacity of the ringbuffer. The returned value could be stale as soon as it is returned.
                 * If ttl is not set, the remaining capacity will always be the capacity.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_remainingcapacity_encode(const std::string &name);

                /**
                 * Adds an item to the tail of the Ringbuffer. If there is space in the ringbuffer, the call
                 * will return the sequence of the written item. If there is no space, it depends on the overflow policy what happens:
                 * OverflowPolicy OVERWRITE we just overwrite the oldest item in the ringbuffer and we violate the ttl
                 * OverflowPolicy FAIL we return -1. The reason that FAIL exist is to give the opportunity to obey the ttl.
                 * <p/>
                 * This sequence will always be unique for this Ringbuffer instance so it can be used as a unique id generator if you are
                 * publishing items on this Ringbuffer. However you need to take care of correctly determining an initial id when any node
                 * uses the ringbuffer for the first time. The most reliable way to do that is to write a dummy item into the ringbuffer and
                 * use the returned sequence as initial  id. On the reading side, this dummy item should be discard. Please keep in mind that
                 * this id is not the sequence of the item you are about to publish but from a previously published item. So it can't be used
                 * to find that item.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_add_encode(const std::string &name, const int32_t &overflowPolicy, const Data &value);

                /**
                 * Reads one item from the Ringbuffer. If the sequence is one beyond the current tail, this call blocks until an
                 * item is added. This method is not destructive unlike e.g. a queue.take. So the same item can be read by multiple
                 * readers or it can be read multiple times by the same reader. Currently it isn't possible to control how long this
                 * call is going to block. In the future we could add e.g. tryReadOne(long sequence, long timeout, TimeUnit unit).
                 */
                 ClientMessage HAZELCAST_API ringbuffer_readone_encode(const std::string &name, const int64_t &sequence);

                /**
                 * Adds all the items of a collection to the tail of the Ringbuffer. A addAll is likely to outperform multiple calls
                 * to add(Object) due to better io utilization and a reduced number of executed operations. If the batch is empty,
                 * the call is ignored. When the collection is not empty, the content is copied into a different data-structure.
                 * This means that: after this call completes, the collection can be re-used. the collection doesn't need to be serializable.
                 * If the collection is larger than the capacity of the ringbuffer, then the items that were written first will be
                 * overwritten. Therefor this call will not block. The items are inserted in the order of the Iterator of the collection.
                 * If an addAll is executed concurrently with an add or addAll, no guarantee is given that items are contiguous.
                 * The result of the future contains the sequenceId of the last written item
                 */
                 ClientMessage HAZELCAST_API ringbuffer_addall_encode(const std::string &name, const std::vector<Data> &valueList, const int32_t &overflowPolicy);

                /**
                 * Reads a batch of items from the Ringbuffer. If the number of available items after the first read item is smaller
                 * than the maxCount, these items are returned. So it could be the number of items read is smaller than the maxCount.
                 * If there are less items available than minCount, then this call blacks. Reading a batch of items is likely to
                 * perform better because less overhead is involved. A filter can be provided to only select items that need to be read.
                 * If the filter is null, all items are read. If the filter is not null, only items where the filter function returns
                 * true are returned. Using filters is a good way to prevent getting items that are of no value to the receiver.
                 * This reduces the amount of IO and the number of operations being executed, and can result in a significant performance improvement.
                 */
                 ClientMessage HAZELCAST_API ringbuffer_readmany_encode(const std::string &name, const int64_t &startSequence, const int32_t &minCount, const int32_t &maxCount, const Data *filter);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API flakeidgenerator_newidbatch_encode(const std::string &name, const int32_t &batchSize);

                /**
                 * Returns true if this map contains an entry for the specified key.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_containskey_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Returns the value for the specified key, or null if this map does not contain this key.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_get_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Locks the key and then gets and returns the value to which the specified key is mapped. Lock will be released at
                 * the end of the transaction (either commit or rollback).
                 */
                 ClientMessage HAZELCAST_API transactionalmap_getforupdate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Returns the number of entries in this map.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Returns true if this map contains no entries.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_isempty_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Associates the specified value with the specified key in this map. If the map previously contained a mapping for
                 * the key, the old value is replaced by the specified value. The object to be put will be accessible only in the
                 * current transaction context till transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_put_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value, const int64_t &ttl);

                /**
                 * Associates the specified value with the specified key in this map. If the map previously contained a mapping for
                 * the key, the old value is replaced by the specified value. This method is preferred to #put(Object, Object)
                 * if the old value is not needed.
                 * The object to be set will be accessible only in the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_set_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * If the specified key is not already associated with a value, associate it with the given value.
                 * The object to be put will be accessible only in the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_putifabsent_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * Replaces the entry for a key only if it is currently mapped to some value. The object to be replaced will be
                 * accessible only in the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_replace_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * Replaces the entry for a key only if currently mapped to a given value. The object to be replaced will be
                 * accessible only in the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_replaceifsame_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &oldValue, const Data &newValue);

                /**
                 * Removes the mapping for a key from this map if it is present. The map will not contain a mapping for the
                 * specified key once the call returns. The object to be removed will be accessible only in the current transaction
                 * context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Removes the mapping for a key from this map if it is present. The map will not contain a mapping for the specified
                 * key once the call returns. This method is preferred to #remove(Object) if the old value is not needed. The object
                 * to be deleted will be removed from only the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_delete_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key);

                /**
                 * Removes the entry for a key only if currently mapped to a given value. The object to be removed will be removed
                 * from only the current transaction context until the transaction is committed.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_removeifsame_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value);

                /**
                 * Returns a set clone of the keys contained in this map. The set is NOT backed by the map, so changes to the map
                 * are NOT reflected in the set, and vice-versa. This method is always executed by a distributed query, so it may throw
                 * a QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_keyset_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Queries the map based on the specified predicate and returns the keys of matching entries. Specified predicate
                 * runs on all members in parallel.The set is NOT backed by the map, so changes to the map are NOT reflected in the
                 * set, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_keysetwithpredicate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &predicate);

                /**
                 * Returns a collection clone of the values contained in this map. The collection is NOT backed by the map,
                 * so changes to the map are NOT reflected in the collection, and vice-versa. This method is always executed by a
                 * distributed query, so it may throw a QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_values_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw
                 * a QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_valueswithpredicate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &predicate);

                /**
                 * Returns true if this map contains an entry for the specified value.
                 */
                 ClientMessage HAZELCAST_API transactionalmap_containsvalue_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &value);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value.Entry will expire and get evicted after the ttl
                 * If ttl is 0, then the entry lives forever.This method returns a clone of the previous value, not the original
                 * (identically equal) value previously put into the map.Time resolution for TTL is seconds. The given TTL value is
                 * rounded to the next closest second value.
                 */
                 ClientMessage HAZELCAST_API map_put_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl);

                /**
                 * This method returns a clone of the original value, so modifying the returned value does not change the actual
                 * value in the map. You should put the modified value back to make changes visible to all nodes.
                 */
                 ClientMessage HAZELCAST_API map_get_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Removes the mapping for a key from this map if it is present (optional operation).
                 * Returns the value to which this map previously associated the key, or null if the map contained no mapping for the key.
                 * If this map permits null values, then a return value of null does not necessarily indicate that the map contained no mapping for the key; it's also
                 * possible that the map explicitly mapped the key to null. The map will not contain a mapping for the specified key once the
                 * call returns.
                 */
                 ClientMessage HAZELCAST_API map_remove_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Replaces the entry for a key only if currently mapped to a given value.
                 */
                 ClientMessage HAZELCAST_API map_replace_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId);

                /**
                 * Replaces the the entry for a key only if existing values equal to the testValue
                 */
                 ClientMessage HAZELCAST_API map_replaceifsame_encode(const std::string &name, const Data &key, const Data &testValue, const Data &value, const int64_t &threadId);

                /**
                 * Returns true if this map contains a mapping for the specified key.
                 */
                 ClientMessage HAZELCAST_API map_containskey_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Returns true if this map maps one or more keys to the specified value.This operation will probably require time
                 * linear in the map size for most implementations of the Map interface.
                 */
                 ClientMessage HAZELCAST_API map_containsvalue_encode(const std::string &name, const Data &value);

                /**
                 * Removes the mapping for a key from this map if existing value equal to the this value
                 */
                 ClientMessage HAZELCAST_API map_removeifsame_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId);

                /**
                 * Removes the mapping for a key from this map if it is present.Unlike remove(Object), this operation does not return
                 * the removed value, which avoids the serialization cost of the returned value.If the removed value will not be used,
                 * a delete operation is preferred over a remove operation for better performance. The map will not contain a mapping
                 * for the specified key once the call returns.
                 * This method breaks the contract of EntryListener. When an entry is removed by delete(), it fires an EntryEvent
                 * with a null oldValue. Also, a listener with predicates will have null values, so only keys can be queried via predicates
                 */
                 ClientMessage HAZELCAST_API map_delete_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * If this map has a MapStore, this method flushes all the local dirty entries by calling MapStore.storeAll()
                 * and/or MapStore.deleteAll().
                 */
                 ClientMessage HAZELCAST_API map_flush_encode(const std::string &name);

                /**
                 * Tries to remove the entry with the given key from this map within the specified timeout value.
                 * If the key is already locked by another thread and/or member, then this operation will wait the timeout
                 * amount for acquiring the lock.
                 */
                 ClientMessage HAZELCAST_API map_tryremove_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &timeout);

                /**
                 * Tries to put the given key and value into this map within a specified timeout value. If this method returns false,
                 * it means that the caller thread could not acquire the lock for the key within the timeout duration,
                 * thus the put operation is not successful.
                 */
                 ClientMessage HAZELCAST_API map_tryput_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &timeout);

                /**
                 * Same as put except that MapStore, if defined, will not be called to store/persist the entry.
                 * If ttl is 0, then the entry lives forever.
                 */
                 ClientMessage HAZELCAST_API map_puttransient_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value if the specified key is not already associated
                 * with a value. Entry will expire and get evicted after the ttl.
                 */
                 ClientMessage HAZELCAST_API map_putifabsent_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value.Entry will expire and get evicted after the ttl
                 * If ttl is 0, then the entry lives forever. Similar to the put operation except that set doesn't
                 * return the old value, which is more efficient.
                 */
                 ClientMessage HAZELCAST_API map_set_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl);

                /**
                 * Acquires the lock for the specified lease time.After lease time, lock will be released.If the lock is not
                 * available then the current thread becomes disabled for thread scheduling purposes and lies dormant until the lock
                 * has been acquired.
                 * Scope of the lock is this map only. Acquired lock is only for the key in this map. Locks are re-entrant,
                 * so if the key is locked N times then it should be unlocked N times before another thread can acquire it.
                 */
                 ClientMessage HAZELCAST_API map_lock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &ttl, const int64_t &referenceId);

                /**
                 * Tries to acquire the lock for the specified key for the specified lease time.After lease time, the lock will be
                 * released.If the lock is not available, then the current thread becomes disabled for thread scheduling
                 * purposes and lies dormant until one of two things happens the lock is acquired by the current thread, or
                 * the specified waiting time elapses.
                 */
                 ClientMessage HAZELCAST_API map_trylock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &lease, const int64_t &timeout, const int64_t &referenceId);

                /**
                 * Checks the lock for the specified key.If the lock is acquired then returns true, else returns false.
                 */
                 ClientMessage HAZELCAST_API map_islocked_encode(const std::string &name, const Data &key);

                /**
                 * Releases the lock for the specified key. It never blocks and returns immediately.
                 * If the current thread is the holder of this lock, then the hold count is decremented.If the hold count is zero,
                 * then the lock is released.  If the current thread is not the holder of this lock,
                 * then ILLEGAL_MONITOR_STATE is thrown.
                 */
                 ClientMessage HAZELCAST_API map_unlock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &referenceId);

                /**
                 * Adds an interceptor for this map. Added interceptor will intercept operations
                 * and execute user defined methods and will cancel operations if user defined method throw exception.
                 */
                 ClientMessage HAZELCAST_API map_addinterceptor_encode(const std::string &name, const Data &interceptor);

                /**
                 * Removes the given interceptor for this map so it will not intercept operations anymore.
                 */
                 ClientMessage HAZELCAST_API map_removeinterceptor_encode(const std::string &name, const std::string &id);

                /**
                 * Adds a MapListener for this map. To receive an event, you should implement a corresponding MapListener
                 * sub-interface for that event.
                 */
                 ClientMessage HAZELCAST_API map_addentrylistenertokeywithpredicate_encode(const std::string &name, const Data &key, const Data &predicate, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addentrylistenertokeywithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key Key for the map entry.
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds an continuous entry listener for this map. Listener will get notified for map add/remove/update/evict events
                 * filtered by the given predicate.
                 */
                 ClientMessage HAZELCAST_API map_addentrylistenerwithpredicate_encode(const std::string &name, const Data &predicate, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addentrylistenerwithpredicate_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds a MapListener for this map. To receive an event, you should implement a corresponding MapListener
                 * sub-interface for that event.
                 */
                 ClientMessage HAZELCAST_API map_addentrylistenertokey_encode(const std::string &name, const Data &key, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key Key for the map entry.
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds a MapListener for this map. To receive an event, you should implement a corresponding MapListener
                 * sub-interface for that event.
                 */
                 ClientMessage HAZELCAST_API map_addentrylistener_encode(const std::string &name, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds an entry listener for this map. Listener will get notified for all map add/remove/update/evict events.
                 */
                 ClientMessage HAZELCAST_API map_addnearcacheentrylistener_encode(const std::string &name, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addnearcacheentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param sourceUuid TODO DOC
                     * @param partitionUuid TODO DOC
                     * @param sequence TODO DOC
                    */
                    virtual void handle_imapinvalidation(const boost::optional<Data> &key, const boost::optional<boost::uuids::uuid> &sourceUuid, const boost::optional<boost::uuids::uuid> &partitionUuid, const int64_t &sequence) = 0;
                    /**
                     * @param keys TODO DOC
                     * @param sourceUuids TODO DOC
                     * @param partitionUuids TODO DOC
                     * @param sequences TODO DOC
                    */
                    virtual void handle_imapbatchinvalidation(const std::vector<Data> &keys, const std::vector<boost::uuids::uuid> &sourceUuids, const std::vector<boost::uuids::uuid> &partitionUuids, const std::vector<int64_t> &sequences) = 0;
                };

                /**
                 * Removes the specified entry listener. Returns silently if there is no such listener added before.
                 */
                 ClientMessage HAZELCAST_API map_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Adds a MapPartitionLostListener. The addPartitionLostListener returns a register-id. This id is needed to remove
                 * the MapPartitionLostListener using the removePartitionLostListener(String) method.
                 * There is no check for duplicate registrations, so if you register the listener twice, it will get events twice.
                 * IMPORTANT: Please see com.hazelcast.partition.PartitionLostListener for weaknesses.
                 * IMPORTANT: Listeners registered from HazelcastClient may miss some of the map partition lost events due
                 * to design limitations.
                 */
                 ClientMessage HAZELCAST_API map_addpartitionlostlistener_encode(const std::string &name, const bool &localOnly);

                struct HAZELCAST_API map_addpartitionlostlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param partitionId TODO DOC
                     * @param uuid TODO DOC
                    */
                    virtual void handle_mappartitionlost(const int32_t &partitionId, const boost::optional<boost::uuids::uuid> &uuid) = 0;
                };

                /**
                 * Removes the specified map partition lost listener. Returns silently if there is no such listener added before.
                 */
                 ClientMessage HAZELCAST_API map_removepartitionlostlistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Returns the EntryView for the specified key.
                 * This method returns a clone of original mapping, modifying the returned value does not change the actual value
                 * in the map. One should put modified value back to make changes visible to all nodes.
                 */
                 ClientMessage HAZELCAST_API map_getentryview_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Evicts the specified key from this map. If a MapStore is defined for this map, then the entry is not deleted
                 * from the underlying MapStore, evict only removes the entry from the memory.
                 */
                 ClientMessage HAZELCAST_API map_evict_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Evicts all keys from this map except the locked ones. If a MapStore is defined for this map, deleteAll is not
                 * called by this method. If you do want to deleteAll to be called use the clear method. The EVICT_ALL event is
                 * fired for any registered listeners.
                 */
                 ClientMessage HAZELCAST_API map_evictall_encode(const std::string &name);

                /**
                 * Loads all keys into the store. This is a batch load operation so that an implementation can optimize the multiple loads.
                 */
                 ClientMessage HAZELCAST_API map_loadall_encode(const std::string &name, const bool &replaceExistingValues);

                /**
                 * Loads the given keys. This is a batch load operation so that an implementation can optimize the multiple loads.
                 */
                 ClientMessage HAZELCAST_API map_loadgivenkeys_encode(const std::string &name, const std::vector<Data> &keys, const bool &replaceExistingValues);

                /**
                 * Returns a set clone of the keys contained in this map. The set is NOT backed by the map, so changes to the map
                 * are NOT reflected in the set, and vice-versa. This method is always executed by a distributed query, so it may
                 * throw a QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_keyset_encode(const std::string &name);

                /**
                 * Returns the entries for the given keys. If any keys are not present in the Map, it will call loadAll The returned
                 * map is NOT backed by the original map, so changes to the original map are NOT reflected in the returned map, and vice-versa.
                 * Please note that all the keys in the request should belong to the partition id to which this request is being sent, all keys
                 * matching to a different partition id shall be ignored. The API implementation using this request may need to send multiple
                 * of these request messages for filling a request for a key set if the keys belong to different partitions.
                 */
                 ClientMessage HAZELCAST_API map_getall_encode(const std::string &name, const std::vector<Data> &keys);

                /**
                 * Returns a collection clone of the values contained in this map.
                 * The collection is NOT backed by the map, so changes to the map are NOT reflected in the collection, and vice-versa.
                 * This method is always executed by a distributed query, so it may throw a QueryResultSizeExceededException
                 * if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_values_encode(const std::string &name);

                /**
                 * Returns a Set clone of the mappings contained in this map.
                 * The collection is NOT backed by the map, so changes to the map are NOT reflected in the collection, and vice-versa.
                 * This method is always executed by a distributed query, so it may throw a QueryResultSizeExceededException
                 * if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_entryset_encode(const std::string &name);

                /**
                 * Queries the map based on the specified predicate and returns the keys of matching entries. Specified predicate
                 * runs on all members in parallel.The set is NOT backed by the map, so changes to the map are NOT reflected in the
                 * set, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_keysetwithpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_valueswithpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * Queries the map based on the specified predicate and returns the matching entries.Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_entrieswithpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * Adds an index to this map for the specified entries so that queries can run faster.If you are querying your values
                 * mostly based on age and active then you should consider indexing these fields.
                 * Index attribute should either have a getter method or be public.You should also make sure to add the indexes before
                 * adding entries to this map.
                 * Indexing time is executed in parallel on each partition by operation threads. The Map is not blocked during this
                 * operation.The time taken in proportional to the size of the Map and the number Members.
                 * Until the index finishes being created, any searches for the attribute will use a full Map scan, thus avoiding
                 * using a partially built index and returning incorrect results.
                 */
                 ClientMessage HAZELCAST_API map_addindex_encode(const std::string &name, const std::string &attribute, const bool &ordered);

                /**
                 * Returns the number of key-value mappings in this map.  If the map contains more than Integer.MAX_VALUE elements,
                 * returns Integer.MAX_VALUE
                 */
                 ClientMessage HAZELCAST_API map_size_encode(const std::string &name);

                /**
                 * Returns true if this map contains no key-value mappings.
                 */
                 ClientMessage HAZELCAST_API map_isempty_encode(const std::string &name);

                /**
                 * Copies all of the mappings from the specified map to this map (optional operation).The effect of this call is
                 * equivalent to that of calling put(Object,Object) put(k, v) on this map once for each mapping from key k to value
                 * v in the specified map.The behavior of this operation is undefined if the specified map is modified while the
                 * operation is in progress.
                 * Please note that all the keys in the request should belong to the partition id to which this request is being sent, all keys
                 * matching to a different partition id shall be ignored. The API implementation using this request may need to send multiple
                 * of these request messages for filling a request for a key set if the keys belong to different partitions.
                 */
                 ClientMessage HAZELCAST_API map_putall_encode(const std::string &name, const std::vector<std::pair<Data, Data>> &entries);

                /**
                 * This method clears the map and invokes MapStore#deleteAll deleteAll on MapStore which, if connected to a database,
                 * will delete the records from that database. The MAP_CLEARED event is fired for any registered listeners.
                 * To clear a map without calling MapStore#deleteAll, use #evictAll.
                 */
                 ClientMessage HAZELCAST_API map_clear_encode(const std::string &name);

                /**
                 * Applies the user defined EntryProcessor to the entry mapped by the key. Returns the the object which is result of
                 * the process() method of EntryProcessor.
                 */
                 ClientMessage HAZELCAST_API map_executeonkey_encode(const std::string &name, const Data &entryProcessor, const Data &key, const int64_t &threadId);

                /**
                 * Applies the user defined EntryProcessor to the entry mapped by the key. Returns immediately with a Future
                 * representing that task.EntryProcessor is not cancellable, so calling Future.cancel() method won't cancel the
                 * operation of EntryProcessor.
                 */
                 ClientMessage HAZELCAST_API map_submittokey_encode(const std::string &name, const Data &entryProcessor, const Data &key, const int64_t &threadId);

                /**
                 * Applies the user defined EntryProcessor to the all entries in the map.Returns the results mapped by each key in the map.
                 */
                 ClientMessage HAZELCAST_API map_executeonallkeys_encode(const std::string &name, const Data &entryProcessor);

                /**
                 * Applies the user defined EntryProcessor to the entries in the map which satisfies provided predicate.
                 * Returns the results mapped by each key in the map.
                 */
                 ClientMessage HAZELCAST_API map_executewithpredicate_encode(const std::string &name, const Data &entryProcessor, const Data &predicate);

                /**
                 * Applies the user defined EntryProcessor to the entries mapped by the collection of keys.The results mapped by
                 * each key in the collection.
                 */
                 ClientMessage HAZELCAST_API map_executeonkeys_encode(const std::string &name, const Data &entryProcessor, const std::vector<Data> &keys);

                /**
                 * Releases the lock for the specified key regardless of the lock owner.It always successfully unlocks the key,
                 * never blocks,and returns immediately.
                 */
                 ClientMessage HAZELCAST_API map_forceunlock_encode(const std::string &name, const Data &key, const int64_t &referenceId);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API map_keysetwithpagingpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * Queries the map based on the specified predicate and returns the values of matching entries. Specified predicate
                 * runs on all members in parallel. The collection is NOT backed by the map, so changes to the map are NOT reflected
                 * in the collection, and vice-versa. This method is always executed by a distributed query, so it may throw a
                 * QueryResultSizeExceededException if query result size limit is configured.
                 */
                 ClientMessage HAZELCAST_API map_valueswithpagingpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API map_entrieswithpagingpredicate_encode(const std::string &name, const Data &predicate);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API map_clearnearcache_encode(const std::string &name, const Address &target);

                /**
                 * Fetches specified number of keys from the specified partition starting from specified table index.
                 */
                 ClientMessage HAZELCAST_API map_fetchkeys_encode(const std::string &name, const int32_t &tableIndex, const int32_t &batch);

                /**
                 * Fetches specified number of entries from the specified partition starting from specified table index.
                 */
                 ClientMessage HAZELCAST_API map_fetchentries_encode(const std::string &name, const int32_t &tableIndex, const int32_t &batch);

                /**
                 * Applies the aggregation logic on all map entries and returns the result
                 */
                 ClientMessage HAZELCAST_API map_aggregate_encode(const std::string &name, const Data &aggregator);

                /**
                 * Applies the aggregation logic on map entries filtered with the Predicate and returns the result
                 */
                 ClientMessage HAZELCAST_API map_aggregatewithpredicate_encode(const std::string &name, const Data &aggregator, const Data &predicate);

                /**
                 * Applies the projection logic on all map entries and returns the result
                 */
                 ClientMessage HAZELCAST_API map_project_encode(const std::string &name, const Data &projection);

                /**
                 * Applies the projection logic on map entries filtered with the Predicate and returns the result
                 */
                 ClientMessage HAZELCAST_API map_projectwithpredicate_encode(const std::string &name, const Data &projection, const Data &predicate);

                /**
                 * Fetches invalidation metadata from partitions of map.
                 */
                 ClientMessage HAZELCAST_API map_fetchnearcacheinvalidationmetadata_encode(const std::vector<std::string> &names, const Address &address);

                /**
                 * TODO DOC
                 */
                 ClientMessage HAZELCAST_API map_assignandgetuuids_encode();

                /**
                 * Removes all entries which match with the supplied predicate
                 */
                 ClientMessage HAZELCAST_API map_removeall_encode(const std::string &name, const Data &predicate);

                /**
                 * Adds listener to map. This listener will be used to listen near cache invalidation events.
                 * Eventually consistent client near caches should use this method to add invalidation listeners
                 * instead of {@link #addNearCacheEntryListener(String, int, boolean)}
                 */
                 ClientMessage HAZELCAST_API map_addnearcacheinvalidationlistener_encode(const std::string &name, const int32_t &listenerFlags, const bool &localOnly);

                struct HAZELCAST_API map_addnearcacheinvalidationlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param sourceUuid TODO DOC
                     * @param partitionUuid TODO DOC
                     * @param sequence TODO DOC
                    */
                    virtual void handle_imapinvalidation(const boost::optional<Data> &key, const boost::optional<boost::uuids::uuid> &sourceUuid, const boost::optional<boost::uuids::uuid> &partitionUuid, const int64_t &sequence) = 0;
                    /**
                     * @param keys TODO DOC
                     * @param sourceUuids TODO DOC
                     * @param partitionUuids TODO DOC
                     * @param sequences TODO DOC
                    */
                    virtual void handle_imapbatchinvalidation(const std::vector<Data> &keys, const std::vector<boost::uuids::uuid> &sourceUuids, const std::vector<boost::uuids::uuid> &partitionUuids, const std::vector<int64_t> &sequences) = 0;
                };

                /**
                 * Fetches the specified number of entries from the specified partition starting from specified table index
                 * that match the predicate and applies the projection logic on them.
                 */
                 ClientMessage HAZELCAST_API map_fetchwithquery_encode(const std::string &name, const int32_t &tableIndex, const int32_t &batch, const Data &projection, const Data &predicate);

                /**
                 * Performs the initial subscription to the map event journal.
                 * This includes retrieving the event journal sequences of the
                 * oldest and newest event in the journal.
                 */
                 ClientMessage HAZELCAST_API map_eventjournalsubscribe_encode(const std::string &name);

                /**
                 * Reads from the map event journal in batches. You may specify the start sequence,
                 * the minumum required number of items in the response, the maximum number of items
                 * in the response, a predicate that the events should pass and a projection to
                 * apply to the events in the journal.
                 * If the event journal currently contains less events than {@code minSize}, the
                 * call will wait until it has sufficient items.
                 * The predicate, filter and projection may be {@code null} in which case all elements are returned
                 * and no projection is applied.
                 */
                 ClientMessage HAZELCAST_API map_eventjournalread_encode(const std::string &name, const int64_t &startSequence, const int32_t &minSize, const int32_t &maxSize, const Data *predicate, const Data *projection);

                /**
                 * Updates TTL (time to live) value of the entry specified by {@code key} with a new TTL value.
                 * New TTL value is valid from this operation is invoked, not from the original creation of the entry.
                 * If the entry does not exist or already expired, then this call has no effect.
                 * <p>
                 * The entry will expire and get evicted after the TTL. If the TTL is 0,
                 * then the entry lives forever. If the TTL is negative, then the TTL
                 * from the map configuration will be used (default: forever).
                 * 
                 * If there is no entry with key {@code key}, this call has no effect.
                 * 
                 * <b>Warning:</b>
                 * <p>
                 * Time resolution for TTL is seconds. The given TTL value is rounded to the next closest second value.
                 */
                 ClientMessage HAZELCAST_API map_setttl_encode(const std::string &name, const Data &key, const int64_t &ttl);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value.Entry will expire and get evicted after the ttl
                 * If ttl is 0, then the entry lives forever.This method returns a clone of the previous value, not the original
                 * (identically equal) value previously put into the map.Time resolution for TTL is seconds. The given TTL value is
                 * rounded to the next closest second value.
                 */
                 ClientMessage HAZELCAST_API map_putwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle);

                /**
                 * Same as put except that MapStore, if defined, will not be called to store/persist the entry.
                 * If ttl and maxIdle are 0, then the entry lives forever.
                 */
                 ClientMessage HAZELCAST_API map_puttransientwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value if the specified key is not already associated
                 * with a value. Entry will expire and get evicted after the ttl or maxIdle, whichever comes first.
                 */
                 ClientMessage HAZELCAST_API map_putifabsentwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle);

                /**
                 * Puts an entry into this map with a given ttl (time to live) value and maxIdle.
                 * Entry will expire and get evicted after the ttl or maxIdle, whichever comes first.
                 * If ttl and maxIdle are 0, then the entry lives forever.
                 * 
                 * Similar to the put operation except that set doesn't return the old value, which is more efficient.
                 */
                 ClientMessage HAZELCAST_API map_setwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle);

                /**
                 * Adds a new item to the transactional list.
                 */
                 ClientMessage HAZELCAST_API transactionallist_add_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item);

                /**
                 * Remove item from the transactional list
                 */
                 ClientMessage HAZELCAST_API transactionallist_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item);

                /**
                 * Returns the size of the list
                 */
                 ClientMessage HAZELCAST_API transactionallist_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId);

                /**
                 * Stores a key-value pair in the multimap.
                 */
                 ClientMessage HAZELCAST_API multimap_put_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId);

                /**
                 * Returns the collection of values associated with the key. The collection is NOT backed by the map, so changes to
                 * the map are NOT reflected in the collection, and vice-versa.
                 */
                 ClientMessage HAZELCAST_API multimap_get_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Removes the given key value pair from the multimap.
                 */
                 ClientMessage HAZELCAST_API multimap_remove_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Returns the set of keys in the multimap.The collection is NOT backed by the map, so changes to the map are NOT
                 * reflected in the collection, and vice-versa.
                 */
                 ClientMessage HAZELCAST_API multimap_keyset_encode(const std::string &name);

                /**
                 * Returns the collection of values in the multimap.The collection is NOT backed by the map, so changes to the map
                 * are NOT reflected in the collection, and vice-versa.
                 */
                 ClientMessage HAZELCAST_API multimap_values_encode(const std::string &name);

                /**
                 * Returns the set of key-value pairs in the multimap.The collection is NOT backed by the map, so changes to the map
                 * are NOT reflected in the collection, and vice-versa
                 */
                 ClientMessage HAZELCAST_API multimap_entryset_encode(const std::string &name);

                /**
                 * Returns whether the multimap contains an entry with the key.
                 */
                 ClientMessage HAZELCAST_API multimap_containskey_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Returns whether the multimap contains an entry with the value.
                 */
                 ClientMessage HAZELCAST_API multimap_containsvalue_encode(const std::string &name, const Data &value);

                /**
                 * Returns whether the multimap contains the given key-value pair.
                 */
                 ClientMessage HAZELCAST_API multimap_containsentry_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId);

                /**
                 * Returns the number of key-value pairs in the multimap.
                 */
                 ClientMessage HAZELCAST_API multimap_size_encode(const std::string &name);

                /**
                 * Clears the multimap. Removes all key-value pairs.
                 */
                 ClientMessage HAZELCAST_API multimap_clear_encode(const std::string &name);

                /**
                 * Returns the number of values that match the given key in the multimap.
                 */
                 ClientMessage HAZELCAST_API multimap_valuecount_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Adds the specified entry listener for the specified key.The listener will be notified for all
                 * add/remove/update/evict events for the specified key only.
                 */
                 ClientMessage HAZELCAST_API multimap_addentrylistenertokey_encode(const std::string &name, const Data &key, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API multimap_addentrylistenertokey_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key The key to listen to
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Adds an entry listener for this multimap. The listener will be notified for all multimap add/remove/update/evict events.
                 */
                 ClientMessage HAZELCAST_API multimap_addentrylistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API multimap_addentrylistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param key TODO DOC
                     * @param value TODO DOC
                     * @param oldValue TODO DOC
                     * @param mergingValue TODO DOC
                     * @param eventType TODO DOC
                     * @param uuid TODO DOC
                     * @param numberOfAffectedEntries TODO DOC
                    */
                    virtual void handle_entry(const boost::optional<Data> &key, const boost::optional<Data> &value, const boost::optional<Data> &oldValue, const boost::optional<Data> &mergingValue, const int32_t &eventType, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &numberOfAffectedEntries) = 0;
                };

                /**
                 * Removes the specified entry listener. Returns silently if no such listener was added before.
                 */
                 ClientMessage HAZELCAST_API multimap_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Acquires the lock for the specified key for the specified lease time. After the lease time, the lock will be
                 * released. If the lock is not available, then the current thread becomes disabled for thread scheduling
                 * purposes and lies dormant until the lock has been acquired. Scope of the lock is for this map only. The acquired
                 * lock is only for the key in this map.Locks are re-entrant, so if the key is locked N times, then it should be
                 * unlocked N times before another thread can acquire it.
                 */
                 ClientMessage HAZELCAST_API multimap_lock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &ttl, const int64_t &referenceId);

                /**
                 * Tries to acquire the lock for the specified key for the specified lease time. After lease time, the lock will be
                 * released. If the lock is not available, then the current thread becomes disabled for thread scheduling purposes
                 * and lies dormant until one of two things happens:the lock is acquired by the current thread, or the specified
                 * waiting time elapses.
                 */
                 ClientMessage HAZELCAST_API multimap_trylock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &lease, const int64_t &timeout, const int64_t &referenceId);

                /**
                 * Checks the lock for the specified key. If the lock is acquired, this method returns true, else it returns false.
                 */
                 ClientMessage HAZELCAST_API multimap_islocked_encode(const std::string &name, const Data &key);

                /**
                 * Releases the lock for the specified key regardless of the lock owner. It always successfully unlocks the key,
                 * never blocks and returns immediately.
                 */
                 ClientMessage HAZELCAST_API multimap_unlock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &referenceId);

                /**
                 * Releases the lock for the specified key regardless of the lock owner. It always successfully unlocks the key,
                 * never blocks and returns immediately.
                 */
                 ClientMessage HAZELCAST_API multimap_forceunlock_encode(const std::string &name, const Data &key, const int64_t &referenceId);

                /**
                 * Removes all the entries with the given key. The collection is NOT backed by the map, so changes to the map are
                 * NOT reflected in the collection, and vice-versa.
                 */
                 ClientMessage HAZELCAST_API multimap_removeentry_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId);

                /**
                 * Removes all the entries with the given key.
                 */
                 ClientMessage HAZELCAST_API multimap_delete_encode(const std::string &name, const Data &key, const int64_t &threadId);

                /**
                 * Inserts the specified element into this queue, waiting up to the specified wait time if necessary for space to
                 * become available.
                 */
                 ClientMessage HAZELCAST_API queue_offer_encode(const std::string &name, const Data &value, const int64_t &timeoutMillis);

                /**
                 * Inserts the specified element into this queue, waiting if necessary for space to become available.
                 */
                 ClientMessage HAZELCAST_API queue_put_encode(const std::string &name, const Data &value);

                /**
                 * Returns the number of elements in this collection.  If this collection contains more than Integer.MAX_VALUE
                 * elements, returns Integer.MAX_VALUE
                 */
                 ClientMessage HAZELCAST_API queue_size_encode(const std::string &name);

                /**
                 * Retrieves and removes the head of this queue.  This method differs from poll only in that it throws an exception
                 * if this queue is empty.
                 */
                 ClientMessage HAZELCAST_API queue_remove_encode(const std::string &name, const Data &value);

                /**
                 * Retrieves and removes the head of this queue, waiting up to the specified wait time if necessary for an element
                 * to become available.
                 */
                 ClientMessage HAZELCAST_API queue_poll_encode(const std::string &name, const int64_t &timeoutMillis);

                /**
                 * Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
                 */
                 ClientMessage HAZELCAST_API queue_take_encode(const std::string &name);

                /**
                 * Retrieves, but does not remove, the head of this queue, or returns null if this queue is empty.
                 */
                 ClientMessage HAZELCAST_API queue_peek_encode(const std::string &name);

                /**
                 * Returns an iterator over the elements in this collection.  There are no guarantees concerning the order in which
                 * the elements are returned (unless this collection is an instance of some class that provides a guarantee).
                 */
                 ClientMessage HAZELCAST_API queue_iterator_encode(const std::string &name);

                /**
                 * Removes all available elements from this queue and adds them to the given collection.  This operation may be more
                 * efficient than repeatedly polling this queue.  A failure encountered while attempting to add elements to
                 * collection c may result in elements being in neither, either or both collections when the associated exception is
                 * thrown. Attempts to drain a queue to itself result in ILLEGAL_ARGUMENT. Further, the behavior of
                 * this operation is undefined if the specified collection is modified while the operation is in progress.
                 */
                 ClientMessage HAZELCAST_API queue_drainto_encode(const std::string &name);

                /**
                 * Removes at most the given number of available elements from this queue and adds them to the given collection.
                 * A failure encountered while attempting to add elements to collection may result in elements being in neither,
                 * either or both collections when the associated exception is thrown. Attempts to drain a queue to itself result in
                 * ILLEGAL_ARGUMENT. Further, the behavior of this operation is undefined if the specified collection is
                 * modified while the operation is in progress.
                 */
                 ClientMessage HAZELCAST_API queue_draintomaxsize_encode(const std::string &name, const int32_t &maxSize);

                /**
                 * Returns true if this queue contains the specified element. More formally, returns true if and only if this queue
                 * contains at least one element e such that value.equals(e)
                 */
                 ClientMessage HAZELCAST_API queue_contains_encode(const std::string &name, const Data &value);

                /**
                 * Return true if this collection contains all of the elements in the specified collection.
                 */
                 ClientMessage HAZELCAST_API queue_containsall_encode(const std::string &name, const std::vector<Data> &dataList);

                /**
                 * Removes all of this collection's elements that are also contained in the specified collection (optional operation).
                 * After this call returns, this collection will contain no elements in common with the specified collection.
                 */
                 ClientMessage HAZELCAST_API queue_compareandremoveall_encode(const std::string &name, const std::vector<Data> &dataList);

                /**
                 * Retains only the elements in this collection that are contained in the specified collection (optional operation).
                 * In other words, removes from this collection all of its elements that are not contained in the specified collection.
                 */
                 ClientMessage HAZELCAST_API queue_compareandretainall_encode(const std::string &name, const std::vector<Data> &dataList);

                /**
                 * Removes all of the elements from this collection (optional operation). The collection will be empty after this
                 * method returns.
                 */
                 ClientMessage HAZELCAST_API queue_clear_encode(const std::string &name);

                /**
                 * Adds all of the elements in the specified collection to this collection (optional operation).The behavior of this
                 * operation is undefined if the specified collection is modified while the operation is in progress.
                 * (This implies that the behavior of this call is undefined if the specified collection is this collection,
                 * and this collection is nonempty.)
                 */
                 ClientMessage HAZELCAST_API queue_addall_encode(const std::string &name, const std::vector<Data> &dataList);

                /**
                 * Adds an listener for this collection. Listener will be notified or all collection add/remove events.
                 */
                 ClientMessage HAZELCAST_API queue_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly);

                struct HAZELCAST_API queue_addlistener_handler : public impl::BaseEventHandler {
                    void handle(ClientMessage &msg);
                    /**
                     * @param item TODO DOC
                     * @param uuid TODO DOC
                     * @param eventType TODO DOC
                    */
                    virtual void handle_item(const boost::optional<Data> &item, const boost::optional<boost::uuids::uuid> &uuid, const int32_t &eventType) = 0;
                };

                /**
                 * Removes the specified item listener.Returns silently if the specified listener was not added before.
                 */
                 ClientMessage HAZELCAST_API queue_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId);

                /**
                 * Returns the number of additional elements that this queue can ideally (in the absence of memory or resource
                 * constraints) accept without blocking, or Integer.MAX_VALUE if there is no intrinsic limit. Note that you cannot
                 * always tell if an attempt to insert an element will succeed by inspecting remainingCapacity because it may be
                 * the case that another thread is about to insert or remove an element.
                 */
                 ClientMessage HAZELCAST_API queue_remainingcapacity_encode(const std::string &name);

                /**
                 * Returns true if this collection contains no elements.
                 */
                 ClientMessage HAZELCAST_API queue_isempty_encode(const std::string &name);

            }
        }
    }
}

