//
// Created by İhsan Demir on 16/02/16.
//

#ifndef HAZELCASTCLIENT_IALLOCATOR_H
#define HAZELCASTCLIENT_IALLOCATOR_H

namespace hazelcast {
    namespace client {
        namespace api {
            enum SerializationType {
                NullType,
                IdentifiedDataSerializable,
                Portable,
                Custom
            };

            class IAllocator {
            public:
                virtual void *allocate(SerializationType type) = 0;

                virtual void deallocate(void *ptr) = 0;
            };

            class DefaultAllocator : public IAllocator {
            public:
                virtual void *allocate(SerializationType type);

                virtual void deallocate(void *ptr);

                static IAllocator *getInstance();
            private:
                static IAllocator *instance;
            };
        }
    }
}

#endif //HAZELCASTCLIENT_IALLOCATOR_H
