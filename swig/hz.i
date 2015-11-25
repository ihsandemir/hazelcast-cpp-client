%module hazelcast

%pragma(java) jniclasspackage="hazelcast.client.jni"

%nspace hazelcast::client::ClientConfig;
%nspace hazelcast::client::HazelcastClient;
%nspace hazelcast::client::IMap;
%nspace hazelcast::client::DistributedObject;
%nspace hazelcast::client::proxy::IMapImpl;
%nspace hazelcast::client::proxy::ProxyImpl;

%{
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/DistributedObject.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/proxy/ProxyImpl.h"

using namespace hazelcast::client;

%}

%include "HazelcastDll.h"

%include "ClientConfig.h"

%include "DistributedObject.h"

%include "EntryView.h"

%include "ProxyImpl.h"

%include "IMapImpl.h"

%include "IMap.h"

%include "HazelcastClient.h"

%template (IntEntryView) hazelcast::client::EntryView<int, int>;

%template (IntMap) hazelcast::client::IMap<int, int>;

%extend hazelcast::client::HazelcastClient {
        %template (getIntMapDistributedObject) getDistributedObject<hazelcast::client::IMap<int, int> >;

        %template (getIntMap) getMap<int, int>;

};






