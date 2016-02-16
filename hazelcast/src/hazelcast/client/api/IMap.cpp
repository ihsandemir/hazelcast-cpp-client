//
// Created by İhsan Demir on 15/02/16.
//

#include "hazelcast/client/api/IMap.h"

namespace hazelcast {
    namespace client {
        namespace api {
            struct Result NewMap::get(const void *key, IAllocator *alloc) {
                Result res;
                res.object = alloc->allocate(IdentifiedDataSerializable);

                // res.serializer = ...; // This shall be retrieved during deserialization getSerializerFor

                return res;
            }

            std::vector<struct Result> NewMap::getAll(void *key, IAllocator *alloc) {
                std::vector<struct Result> results(1);
                results[0].object = alloc->allocate(IdentifiedDataSerializable);

                return results;
            }
        }
    }
}
