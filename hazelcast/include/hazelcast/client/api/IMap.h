//
// Created by İhsan Demir on 15/02/16.
//

#ifndef HAZELCAST_CLIENT_API_IMAP_H
#define HAZELCAST_CLIENT_API_IMAP_H

#include <vector>
#include <hazelcast/client/serialization/Serializer.h>
#include <hazelcast/client/serialization/IdentifiedDataSerializable.h>
#include "IAllocator.h"

namespace hazelcast {
    namespace client {
        namespace api {
            class ParamObject {
            public:
                virtual const ObjectType &getType() = 0;

                template<typename T>
                virtual const T *get() = 0;    // may be replaced by cast operator
            };

            class ResultObject {
                virtual const ObjectType &getType() = 0;

                // This method fills in the provided object using deserialization
                template<typename T>
                virtual T &get(T &alloc) = 0;

                template<typename T>
                virtual const T *get(IAllocator *alloc = NULL) = 0;  // may be replaced by cast operator

                // Releases the ownership of he created object if created
                template<typename T>
                virtual T *release() = 0;
            };


            struct ObjectType {
                int type;
                int factoryId = -1;
                int classId = -1;
            };

            class IMap {
            public:
                virtual ResultObject get(const ParamObject key) = 0;

                virtual std::vector<ResultObject> getAll(const ParamObject key) = 0;

                virtual ResultObject put(const ParamObject key, const ParamObject value) = 0;
            };

            class NewMap : public IMap {
            public:
                struct ObjectType get(const void *key, IAllocator *alloc = DefaultAllocator::getInstance());

                std::vector<struct ObjectType> getAll(void *key, IAllocator *alloc = DefaultAllocator::getInstance());


            };
        }
    }
}

#endif //HAZELCAST_CLIENT_API_IMAP_H
