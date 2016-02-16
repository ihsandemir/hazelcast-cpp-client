//
// Created by İhsan Demir on 16/02/16.
//

#include "hazelcast/client/api/IAllocator.h"

namespace hazelcast {
    namespace client {
        namespace api {
            IAllocator *DefaultAllocator::instance = 0;

            IAllocator *DefaultAllocator::getInstance() {
                // use a better implementation than this
                if (0 == instance) {
                    instance = new DefaultAllocator;
                }
                return instance;
            }


            void *DefaultAllocator::allocate(SerializationType type) {
                return 0;
            }

            void DefaultAllocator::deallocate(void *ptr) {

            }
        }
    }
}
