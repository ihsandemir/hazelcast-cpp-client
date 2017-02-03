#!/usr/bin/env bash

if [ $# -ne 2 ]
then
    echo "Usage: runAllNearCacheTests <interval_in_millis> <map_entry_count>"
    exit 1
fi

INTERVAL_IN_MILLIS=$1
ENTRY_COUNT=$2

START_DIR=`pwd`

START_TIME=`date +%Y%m%d%H%M%S`

CPP_CLIENT_HOME=/home/ihsan/work/src/hazelcast-cpp-client
CPP_CLIENT_BUILD_DIR=${CPP_CLIENT_HOME}/buildSTATIC64Release
CPP_EXE_PATH=${CPP_CLIENT_BUILD_DIR}/hazelcast/test/performance/near-cache/nearCachePerformance
JAVA_CLIENT_HOME=/home/ihsan/work/src/hazelcast/hazelcast-client

cd ${CPP_CLIENT_HOME}

${CPP_CLIENT_HOME}/runAllNearCacheCppTests.sh ${CPP_EXE_PATH} 10.212.1.111 ${INTERVAL_IN_MILLIS} ${ENTRY_COUNT} 2>&1 | tee cppNearCachePerfTestRunLog${START_TIME}.txt

cd ${JAVA_CLIENT_HOME}

${CPP_CLIENT_HOME}/runAllNearCacheJavaTests.sh 10.212.1.111 ${INTERVAL_IN_MILLIS} ${ENTRY_COUNT} 2>&1 | tee javaNearCachePerfTestRunLog${START_TIME}.txt

cd ${START_DIR}

exit 0


