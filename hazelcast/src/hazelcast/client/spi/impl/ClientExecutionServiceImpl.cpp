/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/util/IOUtil.h>
#include <boost/foreach.hpp>
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                ClientExecutionServiceImpl::ClientExecutionServiceImpl(const std::string &name,
                                                                       const ClientProperties &clientProperties,
                                                                       int32_t poolSize)
                        : logger(util::ILogger::getLogger()) {

                    int internalPoolSize = clientProperties.getInternalExecutorPoolSize().getInteger();
                    if (internalPoolSize <= 0) {
                        internalPoolSize = util::IOUtil::to_value<int>(
                                ClientProperties::INTERNAL_EXECUTOR_POOL_SIZE_DEFAULT);
                    }

                    int executorPoolSize = poolSize;
                    if (executorPoolSize <= 0) {
                        executorPoolSize = util::getAvailableCoreCount();
                    }

                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".internal-", internalPoolSize,
                                                                  INT32_MAX));
                    internalExecutor.reset(
                            new util::impl::SimpleExecutorService(logger, name + ".user-", executorPoolSize,
                                                                  INT32_MAX));
                }

                util::ExecutorService &ClientExecutionServiceImpl::getUserExecutor() {
                    return *userExecutor;
                }

                void ClientExecutionServiceImpl::execute(const boost::shared_ptr<util::Runnable> &command) {
                    internalExecutor->execute(command);
                }

                void ClientExecutionServiceImpl::shutdown() {
                    shutdownExecutor("user", *userExecutor, logger);
                    shutdownExecutor("internal", *internalExecutor, logger);

                    repeatingRunners.consume_all(RepeatingRunnerCloser());
                }

                void
                ClientExecutionServiceImpl::shutdownExecutor(const std::string &name, util::ExecutorService &executor,
                                                             util::ILogger &logger) {
                    executor.shutdown();
                    // TODO: implement await
/*
                    try {
                        bool success = executor.awaitTermination(TERMINATE_TIMEOUT_SECONDS, TimeUnit.SECONDS);
                        if (!success) {
                            logger.warning(name + " executor awaitTermination could not complete in " + TERMINATE_TIMEOUT_SECONDS
                                           + " seconds");
                        }
                    } catch (InterruptedException e) {
                        logger.warning(name + " executor await termination is interrupted", e);
                    }
*/

                }

                void
                ClientExecutionServiceImpl::scheduleWithRepetition(const boost::shared_ptr<util::Runnable> &command,
                                                                   int64_t initialDelayInMillis,
                                                                   int64_t periodInMillis) {
                    boost::shared_ptr<RepeatingRunner> command(
                            new RepeatingRunner(command, initialDelayInMillis, periodInMillis));
                    boost::shared_ptr<util::Thread> thread(command);
                    command->setRunnerThread(thread);
                    thread->start();
                    repeatingRunners.push(command);
                }

                ClientExecutionServiceImpl::RepeatingRunner::RepeatingRunner(
                        const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                        int64_t periodInMillis) : command(command), initialDelayInMillis(initialDelayInMillis),
                                                  periodInMillis(periodInMillis), live(true),
                                                  startTimeMillis(util::currentTimeMillis() + initialDelayInMillis) {}

                void ClientExecutionServiceImpl::RepeatingRunner::run() {
                    while (live) {
                        int64_t waitTimeMillis = startTimeMillis - util::currentTimeMillis();
                        if (waitTimeMillis > 0) {
                            if (runnerThread.get()) {
                                runnerThread->interruptibleSleepMillis(waitTimeMillis);
                            } else {
                                util::sleepmillis(waitTimeMillis);
                            }
                        }
                        try {
                            command->run();
                        } catch (exception::IException &e) {
                            util::ILogger::getLogger().warning() << "Repeated runnable " << getName()
                                                                 << " run method caused exception:" << e;
                        }
                    }
                }

                const std::string ClientExecutionServiceImpl::RepeatingRunner::getName() const {
                    return "ClientExecutionServiceImpl::RepeatedRunner";
                }

                void ClientExecutionServiceImpl::RepeatingRunner::shutdown() {
                    live = false;
                }

                void ClientExecutionServiceImpl::RepeatingRunner::setRunnerThread(
                        const boost::shared_ptr<util::Thread> &thread) {
                    runnerThread = thread;
                }

                const boost::shared_ptr<util::Thread> &
                ClientExecutionServiceImpl::RepeatingRunner::getRunnerThread() const {
                    return runnerThread;
                }

                void ClientExecutionServiceImpl::RepeatingRunnerCloser::operator()(
                        const boost::shared_ptr<ClientExecutionServiceImpl::RepeatingRunner> &runner) {
                    runner->shutdown();
                    const boost::shared_ptr<util::Thread> &runnerThread = runner->getRunnerThread();
                    runnerThread->cancel();
                    runnerThread->join();
                }
            }
        }
    }
}
