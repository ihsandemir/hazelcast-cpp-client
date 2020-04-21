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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_

#include <chrono>

#include <boost/thread/executors/basic_thread_pool.hpp>
#include <boost/thread/executors/scheduled_thread_pool.hpp>
#include <boost/thread/executors/scheduler.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class ClientProperties;

        namespace spi {
            namespace impl {
                class HAZELCAST_API ClientExecutionServiceImpl :
                        public std::enable_shared_from_this<ClientExecutionServiceImpl> {
                public:
                    ClientExecutionServiceImpl(const std::string &name, const ClientProperties &properties,
                                               int32_t poolSize);

                    void start();

                    void shutdown();

                    template<typename CompletionToken>
                    void execute(CompletionToken token) {
                        internalExecutor->submit_after(token, boost::chrono::nanoseconds(0));
                    }

                    template<typename CompletionToken>
                    void schedule(CompletionToken token, const boost::chrono::steady_clock::duration &delay) {
                        internalExecutor->submit_after(token, delay);
                    }

                    template<typename CompletionToken>
                    void scheduleWithRepetition(CompletionToken token, const boost::chrono::steady_clock::duration &delay,
                                                const boost::chrono::steady_clock::duration &period) {
                        internalExecutor->submit_after([=]() {
                            token();
                            scheduleRecursive(token, period); }, delay);
                    }

                    boost::basic_thread_pool &getUserExecutor() const;

                private:
                    std::unique_ptr<boost::scheduled_thread_pool> internalExecutor;
                    std::unique_ptr<boost::basic_thread_pool> userExecutor;
                    const ClientProperties &clientProperties;
                    int userExecutorPoolSize;

                    template<typename CompletionToken>
                    void scheduleRecursive(CompletionToken token, const boost::chrono::steady_clock::duration &delay) {
                        internalExecutor->submit_after([=] () {
                            token();
                            scheduleRecursive(token, delay);
                        }, delay);
                    }
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_
