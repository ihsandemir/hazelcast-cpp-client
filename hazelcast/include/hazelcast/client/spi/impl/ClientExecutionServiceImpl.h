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

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/post.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }

    namespace client {
        class ClientProperties;

        namespace spi {
            namespace impl {
                class HAZELCAST_API ClientExecutionServiceImpl :
                        public std::enable_shared_from_this<ClientExecutionServiceImpl> {
                public:
                    ClientExecutionServiceImpl(const std::string &name, const ClientProperties &clientProperties,
                                               int32_t poolSize, util::ILogger &logger);

                    void start();

                    void shutdown();

                    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
                    void execute(BOOST_ASIO_MOVE_ARG(CompletionToken)token) {
                        boost::asio::post(*internalExecutor, token);
                    }

                    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
                    void schedule(BOOST_ASIO_MOVE_ARG(CompletionToken)token,
                                  const std::chrono::steady_clock::duration &delay) {
                        scheduleWithRepetition(token, delay, std::chrono::seconds(0));
                    }

                    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void()) CompletionToken>
                    void scheduleWithRepetition(BOOST_ASIO_MOVE_ARG(CompletionToken)token,
                                                const std::chrono::steady_clock::duration &delay,
                                                const std::chrono::steady_clock::duration &period) {
                        boost::asio::steady_timer timer(*internalExecutor);
                        timer.expires_from_now(delay);
                        timer.async_wait([=]() {
                            token();
                            if (period.count()) {
                                scheduleWithRepetition(token, delay, period);
                            }
                        });
                    }

                    virtual const std::shared_ptr<util::ExecutorService> getUserExecutor() const;

                private:
                    util::ILogger &logger;
                    std::unique_ptr<boost::asio::thread_pool> internalExecutor;
                    std::unique_ptr<boost::asio::thread_pool> userExecutor;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTEXECUTIONSERVICEIMPL_H_
