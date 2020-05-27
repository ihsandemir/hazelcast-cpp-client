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

#include "hazelcast/client/query/Predicates.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace query {
            BasePredicate::BasePredicate(HazelcastClient &client) : outStream(false, &spi::ClientContext(
                    client).getSerializationService().getPortableSerializer()) {}

            NamedPredicate::NamedPredicate(HazelcastClient &client, const std::string &attributeName) : BasePredicate(
                    client) {
                outStream.write(attributeName);
            }

            InstanceOfPredicate::InstanceOfPredicate(HazelcastClient &client, const std::string &javaClassName)
                    : BasePredicate(client) {
                outStream.write(javaClassName);
            }

            SqlPredicate::SqlPredicate(HazelcastClient &client, const std::string &sql)
                    : BasePredicate(client) {
                outStream.write(sql);
            }

            LikePredicate::LikePredicate(HazelcastClient &client, const std::string &attribute,
                                         const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            ILikePredicate::ILikePredicate(HazelcastClient &client, const std::string &attribute,
                                           const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            RegexPredicate::RegexPredicate(HazelcastClient &client, const std::string &attribute,
                                           const std::string &expression) : NamedPredicate(client, attribute) {
                outStream.write(expression);
            }

            TruePredicate::TruePredicate(HazelcastClient &client) : BasePredicate(client) {}

            FalsePredicate::FalsePredicate(HazelcastClient &client) : BasePredicate(client) {}
        }

        namespace serialization {
            constexpr int32_t hz_serializer<query::BetweenPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::BETWEEN_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::EqualPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::EQUAL_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::NotEqualPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::NOTEQUAL_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::GreaterLessPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::GREATERLESS_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::TruePredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::TRUE_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::FalsePredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::FALSE_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::InstanceOfPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::INSTANCEOF_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::LikePredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::LIKE_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::ILikePredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::ILIKE_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::RegexPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::REGEX_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::InPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::IN_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::AndPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::OR_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::OrPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::OR_PREDICATE);
            }

            constexpr int32_t hz_serializer<query::SqlPredicate>::getClassId() noexcept {
                return static_cast<int32_t>(query::PredicateDataSerializerHook::SQL_PREDICATE);
            }
        }
    }
}

