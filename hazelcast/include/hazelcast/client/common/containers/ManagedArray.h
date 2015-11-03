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
// Created by ihsan demir on 25/04/15.
//
/**
 * This class does not take the overhead of vector implementation. It is supposed to be used with
 * fixed length collection of pointers.
 */

#ifndef HAZELCAST_CLIENT_COMMON_CONTAINERS_MANAGEDARRAY_H
#define HAZELCAST_CLIENT_COMMON_CONTAINERS_MANAGEDARRAY_H

#include <stddef.h>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace common {
            namespace containers {
                template <class T>
                class ManagedArray {

                public:
                    // Takes ownership of the buffer area
                    ManagedArray(size_t len, T *buffer) : len(len), buffer(buffer) { }

                    virtual ~ManagedArray() {
                        delete [] buffer;
                    }

                    T &operator[] (size_t len) {
                        return *(buffer+len);
                    }

                    size_t size() const {
                        return len;
                    }

                private:
                    // Prevent copy
                    ManagedArray(const ManagedArray &rhs);
                    ManagedArray &operator = (const ManagedArray &rhs);

                    size_t len;
                    T *buffer;
                };
            }
        }
    }
}





#endif //HAZELCAST_CLIENT_COMMON_CONTAINERS_MANAGEDARRAY_H
