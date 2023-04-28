cmake_minimum_required(VERSION 3.16)

project (HelloWorldITK)

find_package (ITK REQUIRED)
include (${ITK_USE_FILE})
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(SOURCES
    HelpInfo.cpp
    CommandProcessHelper.cpp
    FileProcessHelper.cpp
    HelloWorldITK.cxx
)

add_definitions(-DDEBUG_MODE)
add_executable (HelloWorldITK ${SOURCES})
target_link_libraries (HelloWorldITK PUBLIC ${ITK_LIBRARIES})