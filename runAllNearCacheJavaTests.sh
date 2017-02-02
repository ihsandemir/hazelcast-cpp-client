#!/usr/bin/env bash

if [ $# -ne 2 ]
then
    echo "Usage: runAllNearCacheJavaTests <server ip address> <interval_in_millis>"
    exit 1
fi

SERVER_IP=$1
OPERATION_INTERVAL=$2
NUMBER_OF_MEMBERS=1

DURATION=600000
KEY_SET_SIZE=10000

echo "Testing the java near cache tests against server ${SERVER_IP}  using operation interval ${OPERATION_INTERVAL} milliseconds"

# TEST CASE: no_miss_with_nearcache_1_member_10000keys_10minutes_1msecInterval_4threads
NUM_THREADS=4
USE_NEAR_CACHE="--use-near-cache"
OUT_FILE=java_${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

MAVEN_OPTS="-Xmx4g -Xms4g" mvn exec:java -Dexec.mainClass="com.hazelcast.client.map.impl.nearcache.NearCachePerformanceTest" -Dexec.classpathScope=test  -Dexec.args="--test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE} ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}"

# TEST CASE: no_miss_without_nearcache_1_member_10000keys_10minutes_1msecInterval_4threads
NUM_THREADS=4
USE_NEAR_CACHE=""
OUT_FILE=java_${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

MAVEN_OPTS="-Xmx4g -Xms4g" mvn exec:java -Dexec.mainClass="com.hazelcast.client.map.impl.nearcache.NearCachePerformanceTest" -Dexec.classpathScope=test  -Dexec.args="--test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}"

# TEST CASE: no_miss_with_nearcache_1_member_10000keys_10minutes_1msecInterval_40threads
NUM_THREADS=40
USE_NEAR_CACHE="--use-near-cache"
OUT_FILE=java_${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

MAVEN_OPTS="-Xmx4g -Xms4g" mvn exec:java -Dexec.mainClass="com.hazelcast.client.map.impl.nearcache.NearCachePerformanceTest" -Dexec.classpathScope=test  -Dexec.args="--test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}"

# TEST CASE: no_miss_without_nearcache_1_member_10000keys_10minutes_1msecInterval_40threads
NUM_THREADS=40
USE_NEAR_CACHE=""
OUT_FILE=java_${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

MAVEN_OPTS="-Xmx4g -Xms4g" mvn exec:java -Dexec.mainClass="com.hazelcast.client.map.impl.nearcache.NearCachePerformanceTest" -Dexec.classpathScope=test  -Dexec.args="--test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}"

echo "All Java tests are finished."

exit 0
