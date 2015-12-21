#
# Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
SET(HZ_TESTAPP_NAME identified-data-serializable)

add_executable(${HZ_TESTAPP_NAME} ./Member.cpp)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS}")
	
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	IF(${HZ_LIB_TYPE} MATCHES "STATIC")
		message( STATUS "Shall use the STATIC Hazelcast library for the examples.")
		add_definitions(-DHAZELCAST_USE_STATIC)
	ELSE(${HZ_LIB_TYPE} MATCHES "SHARED")
		message( STATUS "Shall use the SHARED Hazelcast library for the examples.")
		add_definitions(-DHAZELCAST_USE_SHARED)
	ENDIF(${HZ_LIB_TYPE} MATCHES "STATIC")

	set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ} )

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

