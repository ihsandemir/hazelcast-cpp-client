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

#include "hazelcast/client/proxy/TransactionalMapImpl.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        /**
        * Transactional implementation of IMap.
        *
        * @see IMap
        * @param <K> key
        * @param <V> value
        */
        template<typename K, typename V>
        class TransactionalMap : public proxy::TransactionalMapImpl {
            friend class TransactionContext;
        public:
            /**
            * Transactional implementation of IMap#containsKey(Object).
            *
            * @see IMap#containsKey(key)
            */
            boost::future<bool> containsKey(const K& key) {
                return proxy::TransactionalMapImpl::containsKey(toData(&key));
            }

            /**
            * Transactional implementation of IMap#get(Object).
            *
            * @see IMap#get(keu)
            */
            boost::future<boost::optional<V>> get(const K& key) {
                return std::shared_ptr<V>(toObject<V>(proxy::TransactionalMapImpl::getData(toData(&key))));
            }

            /**
            * Transactional implementation of IMap#size().
            *
            * @see IMap#size()
            */
            boost::future<int> size() {
                return proxy::TransactionalMapImpl::size();
            }

            /**
            * Transactional implementation of IMap#isEmpty().
            *
            * @see IMap#isEmpty()
            */
            boost::future<bool> isEmpty() {
                return proxy::TransactionalMapImpl::isEmpty();
            }

            /**
            * Transactional implementation of IMap#put(Object, Object).
            *
            * The object to be put will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#put(key, value)
            */
            boost::future<boost::optional<V>> put(const K& key, const V& value) {
                return std::shared_ptr<V>(toObject<V>(proxy::TransactionalMapImpl::putData(toData(&key), toData(&value))));
            };

            /**
            * Transactional implementation of IMap#set(key, value).
            *
            * The object to be set will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#set(key, value)
            */
            boost::future<void> set(const K& key, const V& value) {
                proxy::TransactionalMapImpl::set(toData(&key), toData(&value));
            }

            /**
            * Transactional implementation of IMap#putIfAbsent(key, value)
            *
            * The object to be put will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#putIfAbsent(key, value)
            */
            boost::future<boost::optional<V>> putIfAbsent(const K& key, const V& value) {
                return std::shared_ptr<V>(toObject<V>(proxy::TransactionalMapImpl::putIfAbsentData(toData(&key), toData(&value))));
            };

            /**
            * Transactional implementation of IMap#replace(key, value).
            *
            * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#replace(key, value)
            */
            boost::future<boost::optional<V>> replace(const K& key, const V& value) {
                return std::shared_ptr<V>(toObject<V>(proxy::TransactionalMapImpl::replaceData(toData(&key), toData(&value))));
            };

            /**
            * Transactional implementation of IMap#replace(key, value, oldValue).
            *
            * The object to be replaced will be accessible only in the current transaction context till transaction is committed.
            *
            * @see IMap#replace(key, value, oldValue)
            */
            boost::future<bool> replace(const K& key, const V& oldValue, const V& newValue) {
                return proxy::TransactionalMapImpl::replace(toData(&key), toData(&oldValue), toData(&newValue));
            };

            /**
            * Transactional implementation of IMap#remove(key).
            *
            * The object to be removed will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#remove(key)
            */
            boost::future<boost::optional<V>> remove(const K& key) {
                return std::shared_ptr<V>(toObject<V>(proxy::TransactionalMapImpl::removeData(toData(&key))));
            };

            /**
            * Transactional implementation of IMap#delete(key).
            *
            * The object to be deleted will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#delete(keu)
            */

            boost::future<void> deleteEntry(const K& key) {
                proxy::TransactionalMapImpl::deleteEntry(toData(&key));
            };

            /**
            * Transactional implementation of IMap#remove(key, value).
            *
            * The object to be removed will be removed from only the current transaction context till transaction is committed.
            *
            * @see IMap#remove(key, value)
            */
            boost::future<bool> remove(const K& key, const V& value) {
                return proxy::TransactionalMapImpl::remove(toData(&key), toData(&value));
            }

            /**
            * Transactional implementation of IMap#keySet().
            *
            *
            * @see IMap#keySet()
            */
            boost::future<std::vector<K>> keySet() {
                return toObjectCollection<K>(proxy::TransactionalMapImpl::keySetData());
            }

            /**
            * Transactional implementation of IMap#keySet(Predicate) .
            *
            *
            * @see IMap#keySet(predicate)
            */
            boost::future<std::vector<K>> keySet(const serialization::IdentifiedDataSerializable *predicate) {
                return toObjectCollection<K>(proxy::TransactionalMapImpl::keySetData(predicate));
            }

            /**
            * Transactional implementation of IMap#values().
            *
            *
            * @see IMap#values()
            */
            boost::future<std::vector<V>> values() {
                return toObjectCollection<K>(proxy::TransactionalMapImpl::valuesData());
            }

            /**
            * Transactional implementation of IMap#values(Predicate) .
            *
            * @see IMap#values(Predicate)
            */
            boost::future<std::vector<V>> values(const serialization::IdentifiedDataSerializable *predicate) {
                return toObjectCollection<K>(proxy::TransactionalMapImpl::valuesData(predicate));
            }

        private:
            TransactionalMap(const std::string& name, txn::TransactionProxy *transactionProxy)
            : proxy::TransactionalMapImpl(name, transactionProxy) {

            }

        };
    }
}

