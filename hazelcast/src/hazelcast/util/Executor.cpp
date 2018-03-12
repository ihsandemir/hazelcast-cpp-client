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

#include <cmath>
#include <boost/foreach.hpp>

#include <hazelcast/client/exception/IException.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/util/HashUtil.h>
#include "hazelcast/util/Executor.h"

namespace hazelcast {
    namespace util {
        int32_t SimpleExecutor::DEFAULT_EXECUTOR_QUEUE_CAPACITY = 100 * 1000;

        SimpleExecutor::SimpleExecutor(ILogger &logger, const std::string &threadNamePrefix, int threadCount,
                                       int32_t maximumQueueCapacity)
                : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                  workers(threadCount), maximumQueueCapacity(maximumQueueCapacity) {
            // `maximumQueueCapacity` is the given max capacity for this executor. Each worker in this executor should consume
            // only a portion of that capacity. Otherwise we will have `threadCount * maximumQueueCapacity` instead of
            // `maximumQueueCapacity`.
            int32_t perThreadMaxQueueCapacity = ceil(1.0D * maximumQueueCapacity / threadCount);
            for (int i = 0; i < threadCount; i++) {
                workers[i].reset(
                        new Worker(<#initializer#>, threadNamePrefix, perThreadMaxQueueCapacity, live, logger));
                workers[i]->start();
            }
        }

        void SimpleExecutor::execute(const boost::shared_ptr<Runnable> &command) {
            if (command.get() == NULL) {
                throw client::exception::IException("SimpleExecutor::execute", "command can't be null");
            }

            if (!live) {
                throw client::exception::RejectedExecutionException("SimpleExecutor::execute",
                                                                    "Executor is terminated!");
            }

            boost::shared_ptr<Worker> worker = getWorker(command);
            worker->schedule(command);
        }

        boost::shared_ptr<SimpleExecutor::Worker>
        SimpleExecutor::getWorker(const boost::shared_ptr<Runnable> &runnable) {
            int32_t key = (int32_t) rand();
            int index = HashUtil::hashToIndex(key, threadCount);
            return workers[index];
        }

        void SimpleExecutor::shutdown() {
            if (!live.compareAndSet(true, false)) {
                return;
            }

            BOOST_FOREACH(boost::shared_ptr<Worker> &worker, workers) {
                            worker->workQueue.clear();
                            worker->wakeup();
                        }
        }
        SimpleExecutor::~SimpleExecutor() {
            shutdown();
        }

        void SimpleExecutor::Worker::run() {
            boost::shared_ptr<Runnable> task;
            while (live) {
                try {
                    task = workQueue.pop();
                    if (task.get()) {
                        task->run();
                    }
                } catch (client::exception::IException &t) {
                    logger.warning() << getName() + " caught an exception" << t;
                }
            }
        }

        const std::string &SimpleExecutor::Worker::getName() const {
            return name;
        }

        SimpleExecutor::Worker::Worker(const std::string &name, const std::string &threadNamePrefix,
                                       int32_t queueCapacity,
                                       util::AtomicBoolean &live, util::ILogger &logger)
                : Thread(threadNamePrefix + "-" + (++THREAD_ID_GENERATOR)), workQueue(queueCapacity), live(live),
                  logger(logger) {
        }

        void SimpleExecutor::Worker::schedule(const boost::shared_ptr<Runnable> &runnable) {
            workQueue.push(runnable);
        }

        StripedExecutor::StripedExecutor(ILogger &logger, const std::string &threadNamePrefix, int32_t threadCount,
                                         int32_t maximumQueueCapacity) : SimpleExecutor(logger, threadNamePrefix,
                                                                                        threadCount,
                                                                                        maximumQueueCapacity) {}

        boost::shared_ptr<SimpleExecutor::Worker>
        StripedExecutor::getWorker(const boost::shared_ptr<StripedRunnable> &runnable) {
            int index = HashUtil::hashToIndex(runnable->getKey(), threadCount);
            return workers[index];
        }


        boost::shared_ptr<ExecutorService> Executors::newSingleThreadExecutor(const std::string &name) {
            return boost::shared_ptr<ExecutorService>(new SimpleExecutor(util::ILogger::getLogger(), name, 1,
                                                                         SimpleExecutor::DEFAULT_EXECUTOR_QUEUE_CAPACITY));
        }
    }
}

