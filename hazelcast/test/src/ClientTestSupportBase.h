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

#include <memory>
#include <thread>
#include <string>
#include <stdint.h>

#include <TestHelperFunctions.h>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/Member.h>
#include <hazelcast/client/spi/ClientContext.h>

namespace hazelcast {
    namespace client {
        class ClientConfig;

        class HazelcastClient;

        namespace test {
            class HazelcastServerFactory;

            extern HazelcastServerFactory *g_srvFactory;

            class ClientTestSupportBase {
            public:
                ClientTestSupportBase();

                static std::string getCAFilePath();

                static std::string randomMapName();

                static std::string randomString();

                static void sleepSeconds(int32_t seconds);

                static boost::uuids::uuid generateKeyOwnedBy(spi::ClientContext &context, const Member &member);
            protected:

                static hazelcast::client::ClientConfig getConfig();

                static HazelcastClient getNewClient();

                static const std::string getSslFilePath();

            };
        }
    }

    namespace util {
        class ThreadArgs {
        public:
            const void *arg0;
            const void *arg1;
            const void *arg2;
            const void *arg3;
            void (*func)(ThreadArgs &);
        };


        class StartedThread {
        public:
            StartedThread(const std::string &name, void (*func)(ThreadArgs &),
                          void *arg0 = nullptr, void *arg1 = nullptr, void *arg2 = nullptr, void *arg3 = nullptr);

            StartedThread(void (func)(ThreadArgs &),
                          void *arg0 = nullptr,
                          void *arg1 = nullptr,
                          void *arg2 = nullptr,
                          void *arg3 = nullptr);

            virtual ~StartedThread();

            bool join();

            virtual void run();

            virtual const std::string getName() const;

        private:
            ThreadArgs threadArgs;
            std::string name;
            std::thread thread;
            std::shared_ptr<util::ILogger> logger;

            void init(void (func)(ThreadArgs &), void *arg0, void *arg1, void *arg2, void *arg3);
        };
    }
}



