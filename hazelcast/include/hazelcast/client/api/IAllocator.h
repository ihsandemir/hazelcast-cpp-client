//
// Created by İhsan Demir on 16/02/16.
//

#ifndef HAZELCASTCLIENT_IALLOCATOR_H
#define HAZELCASTCLIENT_IALLOCATOR_H

#include <stdint.h>

namespace hazelcast {
    namespace client {
        namespace api {
            class IAllocator {
            public:
                /**
                 * type: This uniquely identifies a constructable object. Used in place of class type in Java.
                 * E.g. For primitive types, it is a predefined
                 */
                virtual void *allocate(int32_t type) = 0;

                virtual void deallocate(void *ptr) = 0;
            };

            class DefaultAllocator : public IAllocator {
            public:
                virtual void *allocate(int32_t type);

                virtual void deallocate(void *ptr);

                static IAllocator *getInstance();
            private:
                static IAllocator *instance;
            };
        }
    }
}

#endif //HAZELCASTCLIENT_IALLOCATOR_H
