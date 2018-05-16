SET(BUILD_GMOCK OFF)
SET(BUILD_TEST ON)
#[[
set(CMAKE_MACOSX_RPATH OFF)
SET(BUILD_SHARED_LIBS ON)
]]

add_subdirectory(src)
add_subdirectory(googletest)