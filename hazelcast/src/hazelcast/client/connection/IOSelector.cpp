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
// Created by sancar koyunlu on 24/12/13.
//

#include <string.h>
#include <unistd.h>
#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/util/ServerSocket.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/Thread.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace client {
        namespace connection {

            IOSelector::IOSelector(ConnectionManager &connectionManager)
            :connectionManager(connectionManager) {
                t.tv_sec = 5;
                t.tv_usec = 0;
                isAlive = true;
            }

            void IOSelector::staticListen(util::ThreadArgs &args) {
                IOSelector *inSelector = (IOSelector *) args.arg0;
                inSelector->listen();
            }

            IOSelector::~IOSelector() {
                shutdown();
            }

            void IOSelector::wakeUp() {
                char wakeUpSignal = 9;
                if (write(wakeupFileDescriptors[1], &wakeUpSignal, 1) < 0) {
                    util::ILogger::getLogger().warning("Exception at IOSelector::wakeUp ");
                }
            }

            void IOSelector::listen() {
                while (isAlive) {
                    try{
                        processListenerQueue();
                        listenInternal();
                    }catch(exception::IException &e){
                        util::ILogger::getLogger().warning(std::string("Exception at IOSelector::listen() ") + e.what());
                    }
                }
            }

                bool IOSelector::initListenSocket(util::SocketSet &wakeUpSocketSet) {
                    if (pipe(wakeupFileDescriptors)) {
                        util::ILogger::getLogger().severe("Could not allocate create wakeup pipe!");
                        return false;
                    }

                    wakeUpSocketSet.setWakeUpFd(wakeupFileDescriptors[0]);
                    return true;
                }

            void IOSelector::shutdown() {
                isAlive = false;
                // TODO: Should we close the wakeup pipe descriptors?
            }

            void IOSelector::addTask(ListenerTask *listenerTask) {
                listenerTasks.offer(listenerTask);
            }

            void IOSelector::addSocket(const Socket &socket) {
                socketSet.insertSocket(&socket);
            }

            void IOSelector::removeSocket(const Socket &socket) {
                socketSet.removeSocket(&socket);
            }

            void IOSelector::processListenerQueue() {
                while (ListenerTask *task = listenerTasks.poll()) {
                    task->run();
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



