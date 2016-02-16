//
// Created by İhsan Demir on 15/02/16.
//

#ifndef HAZELCAST_CLIENT_API_IMAP_H
#define HAZELCAST_CLIENT_API_IMAP_H

#include <vector>
#include <hazelcast/client/serialization/Serializer.h>
#include "IAllocator.h"

namespace hazelcast {
    namespace client {
        namespace api {
            /**
             * The object can be cast to the appropriate pointer based on the type id the SerializerBase * identifies.
             * Or it can be cast without this check if we know what the type of the object is in application.
             */
            struct Result {
                void *object;
                const hazelcast::client::serialization::SerializerBase *serializer;
            };

            class IMap {
            public:
                /**
                 * key: Memory address of the the key object
                 * alloc: An allocator implementing the IAllocator interface
                 */
                virtual struct Result get(const void *key, IAllocator *alloc = DefaultAllocator::getInstance()) = 0;

                virtual std::vector<struct Result> getAll(void *key, IAllocator *alloc = DefaultAllocator::getInstance()) = 0;
            };

            class NewMap : public IMap {
            public:
                struct Result get(const void *key, IAllocator *alloc = DefaultAllocator::getInstance());

                std::vector<struct Result> getAll(void *key, IAllocator *alloc = DefaultAllocator::getInstance());
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_API_IMAP_H
