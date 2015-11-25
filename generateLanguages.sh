#!/bin/bash

INCLUDE_DIRS="-Ihazelcast/include/hazelcast/client -Ihazelcast/include/hazelcast/util -Ihazelcast/include/hazelcast/client/proxy"

SWIG_OPTIONS="-c++ -verbose ${INCLUDE_DIRS}"

SWIG_CONFIG_FILE="swig/hz.i"

LANGUAGE=python
#generate language
echo "Generating language ${LANGUAGE}"
rm -rf ${LANGUAGE}
mkdir ${LANGUAGE}
swig -${LANGUAGE} ${SWIG_OPTIONS} -outdir ${LANGUAGE} -o ${LANGUAGE}/hz_python_wrapper.cpp ${SWIG_CONFIG_FILE}

