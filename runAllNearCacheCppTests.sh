#!/usr/bin/env bash

if [ $# -ne 3 ]
then
    echo "Usage: runAllNearCacheCppTests <path of the executable> <server ip address> <interval_in_millis>"
    exit 1
fi

TestExecutable=$1
SERVER_IP=$2
OPERATION_INTERVAL=$3
NUMBER_OF_MEMBERS=1

echo "Testing the executable ${TestExecutable} against server ${SERVER_IP} using operation interval ${OPERATION_INTERVAL} milliseconds"

# TEST CASE: no_miss_with_nearcache_1_member_10000keys_10minutes_1msecInterval_4threads
DURATION=600000
NUM_THREADS=4
KEY_SET_SIZE=10000
USE_NEAR_CACHE="--use-near-cache"
OUT_FILE=${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

${TestExecutable} --test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}

# TEST CASE: no_miss_without_nearcache_1_member_10000keys_10minutes_1msecInterval_4threads
DURATION=600000
NUM_THREADS=4
KEY_SET_SIZE=10000
USE_NEAR_CACHE=""
OUT_FILE=${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

${TestExecutable} --test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}

# TEST CASE: no_miss_with_nearcache_1_member_10000keys_10minutes_1msecInterval_40threads
DURATION=600000
NUM_THREADS=40
KEY_SET_SIZE=10000
USE_NEAR_CACHE="--use-near-cache"
OUT_FILE=${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

${TestExecutable} --test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}

# TEST CASE: no_miss_without_nearcache_1_member_10000keys_10minutes_1msecInterval_40threads
DURATION=600000
NUM_THREADS=40
KEY_SET_SIZE=10000
USE_NEAR_CACHE=""
OUT_FILE=${NUMBER_OF_MEMBERS}member${USE_NEAR_CACHE}${KEY_SET_SIZE}keys_${DURATION}millisecondsDuration_${OPERATION_INTERVAL}msecInterval_${NUM_THREADS}threads.txt

${TestExecutable} --test-duration-in-milliseconds=${DURATION} --num-threads=${NUM_THREADS} --stats-output-file=${OUT_FILE}  ${USE_NEAR_CACHE} --key-set-size=${KEY_SET_SIZE} --server-ip=${SERVER_IP} --operation-interval-in-millis=${OPERATION_INTERVAL}

echo "All C++ tests are finished."

exit 0


