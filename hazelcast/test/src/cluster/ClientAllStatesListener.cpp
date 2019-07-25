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

#include "ClientAllStatesListener.h"

namespace hazelcast {
    namespace client {
        namespace test {
            ClientAllStatesListener::ClientAllStatesListener(util::CountDownLatch *startingLatch,
                                                             util::CountDownLatch *startedLatch,
                                                             util::CountDownLatch *connectedLatch,
                                                             util::CountDownLatch *disconnectedLatch,
                                                             util::CountDownLatch *shuttingDownLatch,
                                                             util::CountDownLatch *shutdownLatch)
                    : startingLatch(startingLatch), startedLatch(startedLatch), connectedLatch(connectedLatch),
                      disconnectedLatch(disconnectedLatch), shuttingDownLatch(shuttingDownLatch),
                      shutdownLatch(shutdownLatch) {}

            void ClientAllStatesListener::stateChanged(const LifecycleEvent &lifecycleEvent) {
                switch (lifecycleEvent.getState()) {
                    case LifecycleEvent::STARTING:
                        if (startingLatch) {
                            startingLatch->countDown();
                        }
                        break;
                    case LifecycleEvent::STARTED:
                        if (startedLatch) {
                            startedLatch->countDown();
                        }
                        break;
                    case LifecycleEvent::CLIENT_CONNECTED:
                        if (connectedLatch) {
                            connectedLatch->countDown();
                        }
                        break;
                    case LifecycleEvent::CLIENT_DISCONNECTED:
                        if (disconnectedLatch) {
                            disconnectedLatch->countDown();
                        }
                        break;
                    case LifecycleEvent::SHUTTING_DOWN:
                        if (shuttingDownLatch) {
                            shuttingDownLatch->countDown();
                        }
                        break;
                    case LifecycleEvent::SHUTDOWN:
                        if (shutdownLatch) {
                            shutdownLatch->countDown();
                        }
                        break;
                    default:
                        FAIL() << "No such state expected:" << lifecycleEvent.getState();
                }
            }
        }
    }
}
