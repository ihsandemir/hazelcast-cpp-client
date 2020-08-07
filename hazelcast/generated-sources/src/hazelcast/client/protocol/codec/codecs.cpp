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

#include <boost/uuid/uuid.hpp>
#include "hazelcast/client/Member.h"

#include "codecs.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                ClientMessage client_authentication_encode(const std::string &clusterName, const std::string *username, const std::string *password, const boost::optional<boost::uuids::uuid> &uuid, const std::string &clientType, const byte &serializationVersion, const std::string &clientHazelcastVersion, const std::string &clientName, const std::vector<std::string> &labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.Authentication");

                    msg.setMessageType(static_cast<int32_t>(256));

                    msg.set(uuid);
                    msg.set(serializationVersion);
                    msg.set(clusterName);

                    msg.setNullable(username);

                    msg.setNullable(password);

                    msg.set(clientType);

                    msg.set(clientHazelcastVersion);

                    msg.set(clientName);

                    msg.set(labels, true);

                    return msg;
                }

                ClientMessage client_authenticationcustom_encode(const std::string &clusterName, const std::vector<byte> &credentials, const boost::optional<boost::uuids::uuid> &uuid, const std::string &clientType, const byte &serializationVersion, const std::string &clientHazelcastVersion, const std::string &clientName, const std::vector<std::string> &labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.AuthenticationCustom");

                    msg.setMessageType(static_cast<int32_t>(512));

                    msg.set(uuid);
                    msg.set(serializationVersion);
                    msg.set(clusterName);

                    msg.set(credentials);

                    msg.set(clientType);

                    msg.set(clientHazelcastVersion);

                    msg.set(clientName);

                    msg.set(labels, true);

                    return msg;
                }

                ClientMessage client_addclusterviewlistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddClusterViewListener");

                    msg.setMessageType(static_cast<int32_t>(768));

                    return msg;
                }

                void client_addclusterviewlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 770) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t version = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        std::vector<Member> memberInfos = msg.get<std::vector<Member>>();

                        handle_membersview(version, memberInfos);
                        return;
                    }
                    if (messageType == 771) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t version = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> partitions = msg.get<std::vector<std::pair<boost::uuids::uuid, std::vector<int>>>>();
                        handle_partitionsview(version, partitions);
                        return;
                    }
                    getLogger()->warning(
                          "[client_addclusterviewlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_createproxy_encode(const std::string &name, const std::string &serviceName) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.CreateProxy");

                    msg.setMessageType(static_cast<int32_t>(1024));

                    msg.set(name);

                    msg.set(serviceName, true);

                    return msg;
                }

                ClientMessage client_destroyproxy_encode(const std::string &name, const std::string &serviceName) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.DestroyProxy");

                    msg.setMessageType(static_cast<int32_t>(1280));

                    msg.set(name);

                    msg.set(serviceName, true);

                    return msg;
                }

                ClientMessage client_addpartitionlostlistener_encode(const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddPartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1536));

                    msg.set(localOnly);
                    return msg;
                }

                void client_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 1538) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t partitionId = msg.get<int32_t>();
                        int32_t lostBackupCount = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> source = msg.get<boost::optional<boost::uuids::uuid>>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        handle_partitionlost(partitionId, lostBackupCount, source);
                        return;
                    }
                    getLogger()->warning(
                          "[client_addpartitionlostlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_removepartitionlostlistener_encode(const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.RemovePartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1792));

                    msg.set(registrationId);
                    return msg;
                }

                ClientMessage client_getdistributedobjects_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.GetDistributedObjects");

                    msg.setMessageType(static_cast<int32_t>(2048));

                    return msg;
                }

                ClientMessage client_adddistributedobjectlistener_encode(const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddDistributedObjectListener");

                    msg.setMessageType(static_cast<int32_t>(2304));

                    msg.set(localOnly);
                    return msg;
                }

                void client_adddistributedobjectlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 2306) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> source = msg.get<boost::optional<boost::uuids::uuid>>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        std::string name = msg.get<std::string>();
                        std::string serviceName = msg.get<std::string>();
                        std::string eventType = msg.get<std::string>();
                        handle_distributedobject(name, serviceName, eventType, source);
                        return;
                    }
                    getLogger()->warning(
                          "[client_adddistributedobjectlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_removedistributedobjectlistener_encode(const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.RemoveDistributedObjectListener");

                    msg.setMessageType(static_cast<int32_t>(2560));

                    msg.set(registrationId);
                    return msg;
                }

                ClientMessage client_ping_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.Ping");

                    msg.setMessageType(static_cast<int32_t>(2816));

                    return msg;
                }

                ClientMessage client_statistics_encode(const int64_t &timestamp, const std::string &clientAttributes, const std::vector<byte> &metricsBlob) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.Statistics");

                    msg.setMessageType(static_cast<int32_t>(3072));

                    msg.set(timestamp);
                    msg.set(clientAttributes);

                    msg.set(metricsBlob, true);

                    return msg;
                }

                ClientMessage client_deployclasses_encode(const std::vector<std::pair<std::string, std::vector<byte>>> &classDefinitions) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.DeployClasses");

                    msg.setMessageType(static_cast<int32_t>(3328));

                    msg.set(classDefinitions, true);

                    return msg;
                }

                ClientMessage client_createproxies_encode(const std::vector<std::pair<std::string, std::string>> &proxies) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.CreateProxies");

                    msg.setMessageType(static_cast<int32_t>(3584));

                    msg.set(proxies, true);

                    return msg;
                }

                ClientMessage client_localbackuplistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.LocalBackupListener");

                    msg.setMessageType(static_cast<int32_t>(3840));

                    return msg;
                }

                void client_localbackuplistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 3842) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int64_t sourceInvocationCorrelationId = msg.get<int64_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT64_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        handle_backup(sourceInvocationCorrelationId);
                        return;
                    }
                    getLogger()->warning(
                          "[client_localbackuplistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_triggerpartitionassignment_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.TriggerPartitionAssignment");

                    msg.setMessageType(static_cast<int32_t>(4096));

                    return msg;
                }

                ClientMessage map_put_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Put");

                    msg.setMessageType(static_cast<int32_t>(65792));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_get_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Get");

                    msg.setMessageType(static_cast<int32_t>(66048));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_remove_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Remove");

                    msg.setMessageType(static_cast<int32_t>(66304));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_replace_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Replace");

                    msg.setMessageType(static_cast<int32_t>(66560));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_replaceifsame_encode(const std::string &name, const Data &key, const Data &testValue, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ReplaceIfSame");

                    msg.setMessageType(static_cast<int32_t>(66816));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(testValue);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_containskey_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(67072));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_containsvalue_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(67328));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_removeifsame_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveIfSame");

                    msg.setMessageType(static_cast<int32_t>(67584));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_delete_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Delete");

                    msg.setMessageType(static_cast<int32_t>(67840));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_flush_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Flush");

                    msg.setMessageType(static_cast<int32_t>(68096));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_tryremove_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.TryRemove");

                    msg.setMessageType(static_cast<int32_t>(68352));

                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_tryput_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.TryPut");

                    msg.setMessageType(static_cast<int32_t>(68608));

                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransient_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutTransient");

                    msg.setMessageType(static_cast<int32_t>(68864));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsent_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutIfAbsent");

                    msg.setMessageType(static_cast<int32_t>(69120));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_set_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Set");

                    msg.setMessageType(static_cast<int32_t>(69376));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_lock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &ttl, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Lock");

                    msg.setMessageType(static_cast<int32_t>(69632));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_trylock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &lease, const int64_t &timeout, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.TryLock");

                    msg.setMessageType(static_cast<int32_t>(69888));

                    msg.set(threadId);
                    msg.set(lease);
                    msg.set(timeout);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_islocked_encode(const std::string &name, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.IsLocked");

                    msg.setMessageType(static_cast<int32_t>(70144));

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_unlock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Unlock");

                    msg.setMessageType(static_cast<int32_t>(70400));

                    msg.set(threadId);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_addinterceptor_encode(const std::string &name, const Data &interceptor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddInterceptor");

                    msg.setMessageType(static_cast<int32_t>(70656));

                    msg.set(name);

                    msg.set(interceptor, true);

                    return msg;
                }

                ClientMessage map_removeinterceptor_encode(const std::string &name, const std::string &id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveInterceptor");

                    msg.setMessageType(static_cast<int32_t>(70912));

                    msg.set(name);

                    msg.set(id, true);

                    return msg;
                }

                ClientMessage map_addentrylistenertokeywithpredicate_encode(const std::string &name, const Data &key, const Data &predicate, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerToKeyWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(71168));

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenertokeywithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71170) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenertokeywithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistenerwithpredicate_encode(const std::string &name, const Data &predicate, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(71424));

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenerwithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71426) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenerwithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistenertokey_encode(const std::string &name, const Data &key, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerToKey");

                    msg.setMessageType(static_cast<int32_t>(71680));

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void map_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71682) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenertokey_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistener_encode(const std::string &name, const bool &includeValue, const int32_t &listenerFlags, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListener");

                    msg.setMessageType(static_cast<int32_t>(71936));

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71938) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(72192));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_addpartitionlostlistener_encode(const std::string &name, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddPartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(72448));

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 72450) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t partitionId = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        handle_mappartitionlost(partitionId, uuid);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addpartitionlostlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_removepartitionlostlistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.RemovePartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(72704));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getentryview_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.GetEntryView");

                    msg.setMessageType(static_cast<int32_t>(72960));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evict_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Evict");

                    msg.setMessageType(static_cast<int32_t>(73216));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evictall_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.EvictAll");

                    msg.setMessageType(static_cast<int32_t>(73472));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadall_encode(const std::string &name, const bool &replaceExistingValues) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.LoadAll");

                    msg.setMessageType(static_cast<int32_t>(73728));

                    msg.set(replaceExistingValues);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadgivenkeys_encode(const std::string &name, const std::vector<Data> &keys, const bool &replaceExistingValues) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.LoadGivenKeys");

                    msg.setMessageType(static_cast<int32_t>(73984));

                    msg.set(replaceExistingValues);
                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_keyset_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.KeySet");

                    msg.setMessageType(static_cast<int32_t>(74240));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getall_encode(const std::string &name, const std::vector<Data> &keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.GetAll");

                    msg.setMessageType(static_cast<int32_t>(74496));

                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_values_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Values");

                    msg.setMessageType(static_cast<int32_t>(74752));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_entryset_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EntrySet");

                    msg.setMessageType(static_cast<int32_t>(75008));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_keysetwithpredicate_encode(const std::string &name, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.KeySetWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75264));

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_valueswithpredicate_encode(const std::string &name, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ValuesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75520));

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_entrieswithpredicate_encode(const std::string &name, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EntriesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75776));

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Size");

                    msg.setMessageType(static_cast<int32_t>(76288));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_isempty_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(76544));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_putall_encode(const std::string &name, const std::vector<std::pair<Data, Data>> &entries, const bool &triggerMapLoader) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutAll");

                    msg.setMessageType(static_cast<int32_t>(76800));

                    msg.set(triggerMapLoader);
                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage map_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Clear");

                    msg.setMessageType(static_cast<int32_t>(77056));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_executeonkey_encode(const std::string &name, const Data &entryProcessor, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnKey");

                    msg.setMessageType(static_cast<int32_t>(77312));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_submittokey_encode(const std::string &name, const Data &entryProcessor, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SubmitToKey");

                    msg.setMessageType(static_cast<int32_t>(77568));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_executeonallkeys_encode(const std::string &name, const Data &entryProcessor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnAllKeys");

                    msg.setMessageType(static_cast<int32_t>(77824));

                    msg.set(name);

                    msg.set(entryProcessor, true);

                    return msg;
                }

                ClientMessage map_executewithpredicate_encode(const std::string &name, const Data &entryProcessor, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(78080));

                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_executeonkeys_encode(const std::string &name, const Data &entryProcessor, const std::vector<Data> &keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnKeys");

                    msg.setMessageType(static_cast<int32_t>(78336));

                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_forceunlock_encode(const std::string &name, const Data &key, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ForceUnlock");

                    msg.setMessageType(static_cast<int32_t>(78592));

                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_fetchkeys_encode(const std::string &name, const std::vector<std::pair<int32_t, int32_t>> &iterationPointers, const int32_t &batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchKeys");

                    msg.setMessageType(static_cast<int32_t>(79616));

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage map_fetchentries_encode(const std::string &name, const std::vector<std::pair<int32_t, int32_t>> &iterationPointers, const int32_t &batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchEntries");

                    msg.setMessageType(static_cast<int32_t>(79872));

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage map_aggregate_encode(const std::string &name, const Data &aggregator) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Aggregate");

                    msg.setMessageType(static_cast<int32_t>(80128));

                    msg.set(name);

                    msg.set(aggregator, true);

                    return msg;
                }

                ClientMessage map_aggregatewithpredicate_encode(const std::string &name, const Data &aggregator, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.AggregateWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(80384));

                    msg.set(name);

                    msg.set(aggregator);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_project_encode(const std::string &name, const Data &projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Project");

                    msg.setMessageType(static_cast<int32_t>(80640));

                    msg.set(name);

                    msg.set(projection, true);

                    return msg;
                }

                ClientMessage map_projectwithpredicate_encode(const std::string &name, const Data &projection, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ProjectWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(80896));

                    msg.set(name);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_fetchnearcacheinvalidationmetadata_encode(const std::vector<std::string> &names, const boost::optional<boost::uuids::uuid> &uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.FetchNearCacheInvalidationMetadata");

                    msg.setMessageType(static_cast<int32_t>(81152));

                    msg.set(uuid);
                    msg.set(names, true);

                    return msg;
                }

                ClientMessage map_removeall_encode(const std::string &name, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveAll");

                    msg.setMessageType(static_cast<int32_t>(81408));

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_addnearcacheinvalidationlistener_encode(const std::string &name, const int32_t &listenerFlags, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddNearCacheInvalidationListener");

                    msg.setMessageType(static_cast<int32_t>(81664));

                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addnearcacheinvalidationlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 81666) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> sourceUuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        boost::optional<boost::uuids::uuid> partitionUuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int64_t sequence = msg.get<int64_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        handle_imapinvalidation(key, sourceUuid, partitionUuid, sequence);
                        return;
                    }
                    if (messageType == 81667) {
                        msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN);

                        std::vector<Data> keys = msg.get<std::vector<Data>>();

                        std::vector<boost::uuids::uuid> sourceUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        std::vector<boost::uuids::uuid> partitionUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        std::vector<int64_t> sequences = msg.get<std::vector<int64_t>>();
                        handle_imapbatchinvalidation(keys, sourceUuids, partitionUuids, sequences);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addnearcacheinvalidationlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_fetchwithquery_encode(const std::string &name, const std::vector<std::pair<int32_t, int32_t>> &iterationPointers, const int32_t &batch, const Data &projection, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchWithQuery");

                    msg.setMessageType(static_cast<int32_t>(81920));

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_eventjournalsubscribe_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EventJournalSubscribe");

                    msg.setMessageType(static_cast<int32_t>(82176));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_eventjournalread_encode(const std::string &name, const int64_t &startSequence, const int32_t &minSize, const int32_t &maxSize, const Data *predicate, const Data *projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EventJournalRead");

                    msg.setMessageType(static_cast<int32_t>(82432));

                    msg.set(startSequence);
                    msg.set(minSize);
                    msg.set(maxSize);
                    msg.set(name);

                    msg.setNullable(predicate);

                    msg.setNullable(projection, true);

                    return msg;
                }

                ClientMessage map_setttl_encode(const std::string &name, const Data &key, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SetTtl");

                    msg.setMessageType(static_cast<int32_t>(82688));

                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_putwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(82944));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransientwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutTransientWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83200));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsentwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutIfAbsentWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83456));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_setwithmaxidle_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId, const int64_t &ttl, const int64_t &maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SetWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83712));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_put_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Put");

                    msg.setMessageType(static_cast<int32_t>(131328));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_get_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Get");

                    msg.setMessageType(static_cast<int32_t>(131584));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_remove_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(131840));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_keyset_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.KeySet");

                    msg.setMessageType(static_cast<int32_t>(132096));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_values_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Values");

                    msg.setMessageType(static_cast<int32_t>(132352));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_entryset_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.EntrySet");

                    msg.setMessageType(static_cast<int32_t>(132608));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_containskey_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(132864));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_containsvalue_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(133120));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_containsentry_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsEntry");

                    msg.setMessageType(static_cast<int32_t>(133376));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Size");

                    msg.setMessageType(static_cast<int32_t>(133632));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Clear");

                    msg.setMessageType(static_cast<int32_t>(133888));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_valuecount_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ValueCount");

                    msg.setMessageType(static_cast<int32_t>(134144));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_addentrylistenertokey_encode(const std::string &name, const Data &key, const bool &includeValue, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.AddEntryListenerToKey");

                    msg.setMessageType(static_cast<int32_t>(134400));

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void multimap_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 134402) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[multimap_addentrylistenertokey_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage multimap_addentrylistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.AddEntryListener");

                    msg.setMessageType(static_cast<int32_t>(134656));

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void multimap_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 134658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t eventType = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t numberOfAffectedEntries = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> key = msg.getNullable<Data>();
                        boost::optional<Data> value = msg.getNullable<Data>();
                        boost::optional<Data> oldValue = msg.getNullable<Data>();
                        boost::optional<Data> mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[multimap_addentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage multimap_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(134912));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_lock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &ttl, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Lock");

                    msg.setMessageType(static_cast<int32_t>(135168));

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_trylock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &lease, const int64_t &timeout, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.TryLock");

                    msg.setMessageType(static_cast<int32_t>(135424));

                    msg.set(threadId);
                    msg.set(lease);
                    msg.set(timeout);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_islocked_encode(const std::string &name, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.IsLocked");

                    msg.setMessageType(static_cast<int32_t>(135680));

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_unlock_encode(const std::string &name, const Data &key, const int64_t &threadId, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Unlock");

                    msg.setMessageType(static_cast<int32_t>(135936));

                    msg.set(threadId);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_forceunlock_encode(const std::string &name, const Data &key, const int64_t &referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ForceUnlock");

                    msg.setMessageType(static_cast<int32_t>(136192));

                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_removeentry_encode(const std::string &name, const Data &key, const Data &value, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.RemoveEntry");

                    msg.setMessageType(static_cast<int32_t>(136448));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_delete_encode(const std::string &name, const Data &key, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Delete");

                    msg.setMessageType(static_cast<int32_t>(136704));

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_putall_encode(const std::string &name, const std::vector<std::pair<Data, std::vector<Data>>> &entries) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.PutAll");

                    msg.setMessageType(static_cast<int32_t>(136960));

                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage queue_offer_encode(const std::string &name, const Data &value, const int64_t &timeoutMillis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Offer");

                    msg.setMessageType(static_cast<int32_t>(196864));

                    msg.set(timeoutMillis);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_put_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Put");

                    msg.setMessageType(static_cast<int32_t>(197120));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Size");

                    msg.setMessageType(static_cast<int32_t>(197376));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remove_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Remove");

                    msg.setMessageType(static_cast<int32_t>(197632));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_poll_encode(const std::string &name, const int64_t &timeoutMillis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Poll");

                    msg.setMessageType(static_cast<int32_t>(197888));

                    msg.set(timeoutMillis);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_take_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Take");

                    msg.setMessageType(static_cast<int32_t>(198144));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_peek_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Peek");

                    msg.setMessageType(static_cast<int32_t>(198400));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_iterator_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Iterator");

                    msg.setMessageType(static_cast<int32_t>(198656));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_drainto_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.DrainTo");

                    msg.setMessageType(static_cast<int32_t>(198912));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_draintomaxsize_encode(const std::string &name, const int32_t &maxSize) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.DrainToMaxSize");

                    msg.setMessageType(static_cast<int32_t>(199168));

                    msg.set(maxSize);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_contains_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Contains");

                    msg.setMessageType(static_cast<int32_t>(199424));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_containsall_encode(const std::string &name, const std::vector<Data> &dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(199680));

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_compareandremoveall_encode(const std::string &name, const std::vector<Data> &dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(199936));

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_compareandretainall_encode(const std::string &name, const std::vector<Data> &dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(200192));

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Clear");

                    msg.setMessageType(static_cast<int32_t>(200448));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_addall_encode(const std::string &name, const std::vector<Data> &dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.AddAll");

                    msg.setMessageType(static_cast<int32_t>(200704));

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.AddListener");

                    msg.setMessageType(static_cast<int32_t>(200960));

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void queue_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 200962) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t eventType = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[queue_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage queue_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Queue.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(201216));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remainingcapacity_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.RemainingCapacity");

                    msg.setMessageType(static_cast<int32_t>(201472));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_isempty_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(201728));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage topic_publish_encode(const std::string &name, const Data &message) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Topic.Publish");

                    msg.setMessageType(static_cast<int32_t>(262400));

                    msg.set(name);

                    msg.set(message, true);

                    return msg;
                }

                ClientMessage topic_addmessagelistener_encode(const std::string &name, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Topic.AddMessageListener");

                    msg.setMessageType(static_cast<int32_t>(262656));

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void topic_addmessagelistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 262658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int64_t publishTime = msg.get<int64_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        Data item = msg.get<Data>();
                        handle_topic(item, publishTime, uuid);
                        return;
                    }
                    getLogger()->warning(
                          "[topic_addmessagelistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage topic_removemessagelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Topic.RemoveMessageListener");

                    msg.setMessageType(static_cast<int32_t>(262912));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Size");

                    msg.setMessageType(static_cast<int32_t>(327936));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_contains_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Contains");

                    msg.setMessageType(static_cast<int32_t>(328192));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_containsall_encode(const std::string &name, const std::vector<Data> &values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(328448));

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_add_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Add");

                    msg.setMessageType(static_cast<int32_t>(328704));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_remove_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Remove");

                    msg.setMessageType(static_cast<int32_t>(328960));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addall_encode(const std::string &name, const std::vector<Data> &valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddAll");

                    msg.setMessageType(static_cast<int32_t>(329216));

                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage list_compareandremoveall_encode(const std::string &name, const std::vector<Data> &values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(329472));

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_compareandretainall_encode(const std::string &name, const std::vector<Data> &values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(329728));

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Clear");

                    msg.setMessageType(static_cast<int32_t>(329984));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_getall_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.GetAll");

                    msg.setMessageType(static_cast<int32_t>(330240));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddListener");

                    msg.setMessageType(static_cast<int32_t>(330496));

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void list_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 330498) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t eventType = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[list_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage list_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(330752));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_isempty_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(331008));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addallwithindex_encode(const std::string &name, const int32_t &index, const std::vector<Data> &valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddAllWithIndex");

                    msg.setMessageType(static_cast<int32_t>(331264));

                    msg.set(index);
                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage list_get_encode(const std::string &name, const int32_t &index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Get");

                    msg.setMessageType(static_cast<int32_t>(331520));

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_set_encode(const std::string &name, const int32_t &index, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Set");

                    msg.setMessageType(static_cast<int32_t>(331776));

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addwithindex_encode(const std::string &name, const int32_t &index, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddWithIndex");

                    msg.setMessageType(static_cast<int32_t>(332032));

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_removewithindex_encode(const std::string &name, const int32_t &index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.RemoveWithIndex");

                    msg.setMessageType(static_cast<int32_t>(332288));

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_lastindexof_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.LastIndexOf");

                    msg.setMessageType(static_cast<int32_t>(332544));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_indexof_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.IndexOf");

                    msg.setMessageType(static_cast<int32_t>(332800));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_sub_encode(const std::string &name, const int32_t &from, const int32_t &to) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Sub");

                    msg.setMessageType(static_cast<int32_t>(333056));

                    msg.set(from);
                    msg.set(to);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_iterator_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Iterator");

                    msg.setMessageType(static_cast<int32_t>(333312));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_listiterator_encode(const std::string &name, const int32_t &index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.ListIterator");

                    msg.setMessageType(static_cast<int32_t>(333568));

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Size");

                    msg.setMessageType(static_cast<int32_t>(393472));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_contains_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Contains");

                    msg.setMessageType(static_cast<int32_t>(393728));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_containsall_encode(const std::string &name, const std::vector<Data> &items) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(393984));

                    msg.set(name);

                    msg.set(items, true);

                    return msg;
                }

                ClientMessage set_add_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Add");

                    msg.setMessageType(static_cast<int32_t>(394240));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_remove_encode(const std::string &name, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Remove");

                    msg.setMessageType(static_cast<int32_t>(394496));

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_addall_encode(const std::string &name, const std::vector<Data> &valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.AddAll");

                    msg.setMessageType(static_cast<int32_t>(394752));

                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage set_compareandremoveall_encode(const std::string &name, const std::vector<Data> &values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(395008));

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_compareandretainall_encode(const std::string &name, const std::vector<Data> &values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(395264));

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Clear");

                    msg.setMessageType(static_cast<int32_t>(395520));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_getall_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.GetAll");

                    msg.setMessageType(static_cast<int32_t>(395776));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_addlistener_encode(const std::string &name, const bool &includeValue, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.AddListener");

                    msg.setMessageType(static_cast<int32_t>(396032));

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void set_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 396034) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int32_t eventType = msg.get<int32_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        boost::optional<Data> item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[set_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage set_removelistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Set.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(396288));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_isempty_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(396544));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_containskey_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(917760));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_get_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Get");

                    msg.setMessageType(static_cast<int32_t>(918016));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_getforupdate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.GetForUpdate");

                    msg.setMessageType(static_cast<int32_t>(918272));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Size");

                    msg.setMessageType(static_cast<int32_t>(918528));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_isempty_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(918784));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_put_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value, const int64_t &ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Put");

                    msg.setMessageType(static_cast<int32_t>(919040));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_set_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Set");

                    msg.setMessageType(static_cast<int32_t>(919296));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_putifabsent_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.PutIfAbsent");

                    msg.setMessageType(static_cast<int32_t>(919552));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replace_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Replace");

                    msg.setMessageType(static_cast<int32_t>(919808));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replaceifsame_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &oldValue, const Data &newValue) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ReplaceIfSame");

                    msg.setMessageType(static_cast<int32_t>(920064));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(oldValue);

                    msg.set(newValue, true);

                    return msg;
                }

                ClientMessage transactionalmap_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(920320));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_delete_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Delete");

                    msg.setMessageType(static_cast<int32_t>(920576));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_removeifsame_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.RemoveIfSame");

                    msg.setMessageType(static_cast<int32_t>(920832));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_keyset_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.KeySet");

                    msg.setMessageType(static_cast<int32_t>(921088));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_keysetwithpredicate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.KeySetWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(921344));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_values_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Values");

                    msg.setMessageType(static_cast<int32_t>(921600));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_valueswithpredicate_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ValuesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(921856));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_containsvalue_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(922112));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_put_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Put");

                    msg.setMessageType(static_cast<int32_t>(983296));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_get_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Get");

                    msg.setMessageType(static_cast<int32_t>(983552));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(983808));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_removeentry_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.RemoveEntry");

                    msg.setMessageType(static_cast<int32_t>(984064));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_valuecount_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.ValueCount");

                    msg.setMessageType(static_cast<int32_t>(984320));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Size");

                    msg.setMessageType(static_cast<int32_t>(984576));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalset_add_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Add");

                    msg.setMessageType(static_cast<int32_t>(1048832));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Remove");

                    msg.setMessageType(static_cast<int32_t>(1049088));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Size");

                    msg.setMessageType(static_cast<int32_t>(1049344));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionallist_add_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Add");

                    msg.setMessageType(static_cast<int32_t>(1114368));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_remove_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Remove");

                    msg.setMessageType(static_cast<int32_t>(1114624));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Size");

                    msg.setMessageType(static_cast<int32_t>(1114880));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_offer_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const Data &item, const int64_t &timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Offer");

                    msg.setMessageType(static_cast<int32_t>(1179904));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalqueue_take_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Take");

                    msg.setMessageType(static_cast<int32_t>(1180160));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_poll_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const int64_t &timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Poll");

                    msg.setMessageType(static_cast<int32_t>(1180416));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_peek_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId, const int64_t &timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Peek");

                    msg.setMessageType(static_cast<int32_t>(1180672));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_size_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &txnId, const int64_t &threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Size");

                    msg.setMessageType(static_cast<int32_t>(1180928));

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_clear_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Clear");

                    msg.setMessageType(static_cast<int32_t>(1245696));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_removeallkeys_encode(const std::string &name, const std::vector<Data> &keys, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.RemoveAllKeys");

                    msg.setMessageType(static_cast<int32_t>(1245952));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage cache_removeall_encode(const std::string &name, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.RemoveAll");

                    msg.setMessageType(static_cast<int32_t>(1246208));

                    msg.set(completionId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_containskey_encode(const std::string &name, const Data &key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(1246464));

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage cache_destroy_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Destroy");

                    msg.setMessageType(static_cast<int32_t>(1246976));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_entryprocessor_encode(const std::string &name, const Data &key, const Data &entryProcessor, const std::vector<Data> &arguments, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.EntryProcessor");

                    msg.setMessageType(static_cast<int32_t>(1247232));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(entryProcessor);

                    msg.set(arguments, true);

                    return msg;
                }

                ClientMessage cache_getall_encode(const std::string &name, const std::vector<Data> &keys, const Data *expiryPolicy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.GetAll");

                    msg.setMessageType(static_cast<int32_t>(1247488));

                    msg.set(name);

                    msg.set(keys);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_getandremove_encode(const std::string &name, const Data &key, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.GetAndRemove");

                    msg.setMessageType(static_cast<int32_t>(1247744));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage cache_getandreplace_encode(const std::string &name, const Data &key, const Data &value, const Data *expiryPolicy, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.GetAndReplace");

                    msg.setMessageType(static_cast<int32_t>(1248000));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_getconfig_encode(const std::string &name, const std::string &simpleName) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.GetConfig");

                    msg.setMessageType(static_cast<int32_t>(1248256));

                    msg.set(name);

                    msg.set(simpleName, true);

                    return msg;
                }

                ClientMessage cache_get_encode(const std::string &name, const Data &key, const Data *expiryPolicy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.Get");

                    msg.setMessageType(static_cast<int32_t>(1248512));

                    msg.set(name);

                    msg.set(key);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_iterate_encode(const std::string &name, const std::vector<std::pair<int32_t, int32_t>> &iterationPointers, const int32_t &batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Iterate");

                    msg.setMessageType(static_cast<int32_t>(1248768));

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage cache_listenerregistration_encode(const std::string &name, const Data &listenerConfig, const bool &shouldRegister, const boost::optional<boost::uuids::uuid> &uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.ListenerRegistration");

                    msg.setMessageType(static_cast<int32_t>(1249024));

                    msg.set(shouldRegister);
                    msg.set(uuid);
                    msg.set(name);

                    msg.set(listenerConfig, true);

                    return msg;
                }

                ClientMessage cache_loadall_encode(const std::string &name, const std::vector<Data> &keys, const bool &replaceExistingValues) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.LoadAll");

                    msg.setMessageType(static_cast<int32_t>(1249280));

                    msg.set(replaceExistingValues);
                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage cache_managementconfig_encode(const std::string &name, const bool &isStat, const bool &enabled, const boost::optional<boost::uuids::uuid> &uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.ManagementConfig");

                    msg.setMessageType(static_cast<int32_t>(1249536));

                    msg.set(isStat);
                    msg.set(enabled);
                    msg.set(uuid);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_putifabsent_encode(const std::string &name, const Data &key, const Data &value, const Data *expiryPolicy, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.PutIfAbsent");

                    msg.setMessageType(static_cast<int32_t>(1249792));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_put_encode(const std::string &name, const Data &key, const Data &value, const Data *expiryPolicy, const bool &get, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Put");

                    msg.setMessageType(static_cast<int32_t>(1250048));

                    msg.set(get);
                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_removeentrylistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(1250304));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_removeinvalidationlistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.RemoveInvalidationListener");

                    msg.setMessageType(static_cast<int32_t>(1250560));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_remove_encode(const std::string &name, const Data &key, const Data *currentValue, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Remove");

                    msg.setMessageType(static_cast<int32_t>(1250816));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.setNullable(currentValue, true);

                    return msg;
                }

                ClientMessage cache_replace_encode(const std::string &name, const Data &key, const Data *oldValue, const Data &newValue, const Data *expiryPolicy, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.Replace");

                    msg.setMessageType(static_cast<int32_t>(1251072));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(key);

                    msg.setNullable(oldValue);

                    msg.set(newValue);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.Size");

                    msg.setMessageType(static_cast<int32_t>(1251328));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_addpartitionlostlistener_encode(const std::string &name, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.AddPartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1251584));

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void cache_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 1251586) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        int32_t partitionId = msg.get<int32_t>();
                        boost::optional<boost::uuids::uuid> uuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::INT32_SIZE + ClientMessage::UUID_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        handle_cachepartitionlost(partitionId, uuid);
                        return;
                    }
                    getLogger()->warning(
                          "[cache_addpartitionlostlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage cache_removepartitionlostlistener_encode(const std::string &name, const boost::optional<boost::uuids::uuid> &registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.RemovePartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1251840));

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_putall_encode(const std::string &name, const std::vector<std::pair<Data, Data>> &entries, const Data *expiryPolicy, const int32_t &completionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.PutAll");

                    msg.setMessageType(static_cast<int32_t>(1252096));

                    msg.set(completionId);
                    msg.set(name);

                    msg.set(entries);

                    msg.setNullable(expiryPolicy, true);

                    return msg;
                }

                ClientMessage cache_iterateentries_encode(const std::string &name, const std::vector<std::pair<int32_t, int32_t>> &iterationPointers, const int32_t &batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.IterateEntries");

                    msg.setMessageType(static_cast<int32_t>(1252352));

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage cache_addnearcacheinvalidationlistener_encode(const std::string &name, const bool &localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.AddNearCacheInvalidationListener");

                    msg.setMessageType(static_cast<int32_t>(1252608));

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void cache_addnearcacheinvalidationlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 1252610) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN));
                        boost::optional<boost::uuids::uuid> sourceUuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        boost::optional<boost::uuids::uuid> partitionUuid = msg.get<boost::optional<boost::uuids::uuid>>();
                        int64_t sequence = msg.get<int64_t>();
                        auto remaining_initial_frame_bytes = initial_frame->frame_len - ClientMessage::RESPONSE_HEADER_LEN;
                        remaining_initial_frame_bytes -= (ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE);
                        // skip any remaining bytes to the end of the frame
                        msg.rd_ptr(remaining_initial_frame_bytes);

                        std::string name = msg.get<std::string>();
                        boost::optional<Data> key = msg.getNullable<Data>();
                        handle_cacheinvalidation(name, key, sourceUuid, partitionUuid, sequence);
                        return;
                    }
                    if (messageType == 1252611) {
                        msg.rd_ptr(ClientMessage::RESPONSE_HEADER_LEN);

                        std::string name = msg.get<std::string>();
                        std::vector<Data> keys = msg.get<std::vector<Data>>();

                        std::vector<boost::uuids::uuid> sourceUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        std::vector<boost::uuids::uuid> partitionUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        std::vector<int64_t> sequences = msg.get<std::vector<int64_t>>();
                        handle_cachebatchinvalidation(name, keys, sourceUuids, partitionUuids, sequences);
                        return;
                    }
                    getLogger()->warning(
                          "[cache_addnearcacheinvalidationlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage cache_fetchnearcacheinvalidationmetadata_encode(const std::vector<std::string> &names, const boost::optional<boost::uuids::uuid> &uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.FetchNearCacheInvalidationMetadata");

                    msg.setMessageType(static_cast<int32_t>(1252864));

                    msg.set(uuid);
                    msg.set(names, true);

                    return msg;
                }

                ClientMessage cache_eventjournalsubscribe_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.EventJournalSubscribe");

                    msg.setMessageType(static_cast<int32_t>(1253120));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cache_eventjournalread_encode(const std::string &name, const int64_t &startSequence, const int32_t &minSize, const int32_t &maxSize, const Data *predicate, const Data *projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Cache.EventJournalRead");

                    msg.setMessageType(static_cast<int32_t>(1253376));

                    msg.set(startSequence);
                    msg.set(minSize);
                    msg.set(maxSize);
                    msg.set(name);

                    msg.setNullable(predicate);

                    msg.setNullable(projection, true);

                    return msg;
                }

                ClientMessage cache_setexpirypolicy_encode(const std::string &name, const std::vector<Data> &keys, const Data &expiryPolicy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Cache.SetExpiryPolicy");

                    msg.setMessageType(static_cast<int32_t>(1253632));

                    msg.set(name);

                    msg.set(keys);

                    msg.set(expiryPolicy, true);

                    return msg;
                }

                ClientMessage xatransaction_collecttransactions_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("XATransaction.CollectTransactions");

                    msg.setMessageType(static_cast<int32_t>(1311232));

                    return msg;
                }

                ClientMessage xatransaction_commit_encode(const boost::optional<boost::uuids::uuid> &transactionId, const bool &onePhase) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("XATransaction.Commit");

                    msg.setMessageType(static_cast<int32_t>(1311744));

                    msg.set(transactionId);
                    msg.set(onePhase);
                    return msg;
                }

                ClientMessage xatransaction_prepare_encode(const boost::optional<boost::uuids::uuid> &transactionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("XATransaction.Prepare");

                    msg.setMessageType(static_cast<int32_t>(1312256));

                    msg.set(transactionId);
                    return msg;
                }

                ClientMessage xatransaction_rollback_encode(const boost::optional<boost::uuids::uuid> &transactionId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("XATransaction.Rollback");

                    msg.setMessageType(static_cast<int32_t>(1312512));

                    msg.set(transactionId);
                    return msg;
                }

                ClientMessage ringbuffer_size_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.Size");

                    msg.setMessageType(static_cast<int32_t>(1507584));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_tailsequence_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.TailSequence");

                    msg.setMessageType(static_cast<int32_t>(1507840));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_headsequence_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.HeadSequence");

                    msg.setMessageType(static_cast<int32_t>(1508096));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_capacity_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.Capacity");

                    msg.setMessageType(static_cast<int32_t>(1508352));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_remainingcapacity_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.RemainingCapacity");

                    msg.setMessageType(static_cast<int32_t>(1508608));

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_add_encode(const std::string &name, const int32_t &overflowPolicy, const Data &value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Ringbuffer.Add");

                    msg.setMessageType(static_cast<int32_t>(1508864));

                    msg.set(overflowPolicy);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage ringbuffer_readone_encode(const std::string &name, const int64_t &sequence) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.ReadOne");

                    msg.setMessageType(static_cast<int32_t>(1509120));

                    msg.set(sequence);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_addall_encode(const std::string &name, const std::vector<Data> &valueList, const int32_t &overflowPolicy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Ringbuffer.AddAll");

                    msg.setMessageType(static_cast<int32_t>(1509376));

                    msg.set(overflowPolicy);
                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage ringbuffer_readmany_encode(const std::string &name, const int64_t &startSequence, const int32_t &minCount, const int32_t &maxCount, const Data *filter) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.ReadMany");

                    msg.setMessageType(static_cast<int32_t>(1509632));

                    msg.set(startSequence);
                    msg.set(minCount);
                    msg.set(maxCount);
                    msg.set(name);

                    msg.setNullable(filter, true);

                    return msg;
                }

                ClientMessage cardinalityestimator_add_encode(const std::string &name, const int64_t &hash) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("CardinalityEstimator.Add");

                    msg.setMessageType(static_cast<int32_t>(1638656));

                    msg.set(hash);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cardinalityestimator_estimate_encode(const std::string &name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("CardinalityEstimator.Estimate");

                    msg.setMessageType(static_cast<int32_t>(1638912));

                    msg.set(name, true);

                    return msg;
                }

            }
        }
    }
}

