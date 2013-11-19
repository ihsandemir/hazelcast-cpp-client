//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_MultiExecutionCallbackWrapper
#define HAZELCAST_MultiExecutionCallbackWrapper

#include "hazelcast/client/connection/Member.h"
#include <boost/atomic.hpp>

namespace hazelcast {
    namespace client {
        namespace impl {
            template <typename Result, typename MultiExecutionCallback>
            class MultiExecutionCallbackWrapper {
            public:
                MultiExecutionCallbackWrapper(int membersSize, MultiExecutionCallback& callback)
                :memberCount(membersSize)
                , multiExecutionCallback(callback) {

                };

                void onResponse(const connection::Member& member, const Result& result) {
                    multiExecutionCallback.onResponse(member, result);
                    values[member] = result;
                    int waitingResponse = memberCount--;
                    if (waitingResponse == 1) {
                        multiExecutionCallback.onComplete(values);
                    }
                }

            private:
                std::map <connection::Member, Result> values;
                boost::atomic<int> memberCount;
                MultiExecutionCallback& multiExecutionCallback;

            };
        }
    }
}
#endif //HAZELCAST_MultiExecutionCallbackWrapper