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
// Created by sancar koyunlu on 31/03/14.
//

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

#include "hazelcast/client/exception/TimeoutException.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/client/exception/pimpl/ExceptionHandler.h"
#include "hazelcast/util/ILogger.h"
#include <memory>
#include <cassert>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Future {
        public:
            Future() {
                sharedObject.store(NULL, std::memory_order_release);
                exceptionReady.store(false, std::memory_order_release);
            };

            void set_value(T *value) {
                sharedObject.store(value, std::memory_order_release);
            };

            void set_exception(const std::string &name, const std::string &details) {

                exceptionReady.store(true, std::memory_order_release);
            };

            T get() {
                T *object;
                std::string exc;
                do {
                    object = sharedObject.load(std::memory_order_consume);
                    if (NULL != object) {
                        return std::auto_ptr<T>(object);
                    } else {
                        if (exceptionReady.load(std::memory_order_consume)) {
                            client::exception::pimpl::ExceptionHandler::rethrow("", "");
                        }
                    }
                    usleep(1);
                } while (true);
            };

            T *get(time_t timeInSeconds) {
                T *object;
                std::string exc;
                time_t endTime = time(NULL) + timeInSeconds;
                do {
                    object = sharedObject.load(std::memory_order_consume);
                    if (NULL != object) {
                        return object;
                    } else {
                        if (exceptionReady.load(std::memory_order_consume)) {
                            client::exception::pimpl::ExceptionHandler::rethrow("", "");
                        }
                    }
                    if (time(NULL) < endTime) {
                        throw client::exception::TimeoutException("Future::get(timeInSeconds)", "Wait is timed out");
                    }
                    usleep(1);
                } while (true);
            };

            void reset() {
                sharedObject.store(NULL, std::memory_order_release);
                exceptionReady.store(false, std::memory_order_release);
            }

        private:
            //std::atomic_flag resultReady;
            //std::atomic_flag exceptionReady;
            //ConditionVariable conditionVariable;
            //Mutex mutex;
            std::atomic<T *> sharedObject;
            std::atomic_bool exceptionReady;
            //std::atomic<std::string exceptionName;
            //std::atomic<std::string> exceptionDetails;

            Future(const Future &rhs);

            void operator=(const Future &rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Future

