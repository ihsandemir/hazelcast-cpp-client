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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_

#include <memory>

#include <hazelcast/util/Future.h>
#include <hazelcast/client/protocol/ClientMessage.h>
#include <boost/foreach.hpp>
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/impl/ExecutionCallback.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                class ClientInvocation;

                class HAZELCAST_API ClientInvocationFuture
                        : public util::Future<const boost::shared_ptr<protocol::ClientMessage> > {
                public:
                    ClientInvocationFuture(util::ILogger &logger, ClientInvocation &invocation,
                                           sequence::CallIdSequence &callIdSequence, util::Executor &defaultExecutor)
                            : logger(logger),
                              invocation(invocation),
                              callIdSequence(callIdSequence), defaultExecutor(defaultExecutor) {}

                    virtual void onComplete() {
                        callIdSequence.complete();
                    }

                    ClientInvocation &getInvocation() const {
                        return invocation;
                    }

                    void
                    andThen(const boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > &callback) {
                        util::Future<boost::shared_ptr<protocol::ClientMessage> >::andThen(callback, defaultExecutor);
                    }

                private:
                    class InternalDelegatingExecutionCallback
                            : public client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > {
                    public:
                        InternalDelegatingExecutionCallback(
                                const boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > &callback,
                                sequence::CallIdSequence &callIdSequence) : callback(callback),
                                                                            callIdSequence(callIdSequence) {
                            this->callIdSequence.forceNext();
                        }

                        virtual void onResponse(boost::shared_ptr<protocol::ClientMessage> &message) {
                            try {
                                callback->onResponse(message);
                                callIdSequence.complete();
                            } catch (...) {
                                callIdSequence.complete();
                            }
                        }

                        virtual void onFailure(const exception::IException &e) {
                            try {
                                callback->onFailure(e);
                                callIdSequence.complete();
                            } catch (...) {
                                callIdSequence.complete();
                            }
                        }

                    private:
                        boost::shared_ptr<client::impl::ExecutionCallback<boost::shared_ptr<protocol::ClientMessage> > > callback;
                        sequence::CallIdSequence &callIdSequence;
                    };


                    util::ILogger &logger;
                    ClientInvocation &invocation;
                    sequence::CallIdSequence &callIdSequence;
                    util::Executor &defaultExecutor;
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATIONFUTURE_H_
