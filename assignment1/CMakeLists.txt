cmake_minimum_required(VERSION 2.8)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


SET (CMAKE_BUILD_TYPE "Release")
#SET (CMAKE_BUILD_TYPE "Debug")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET (BASE_DIR ${CMAKE_SOURCE_DIR} )
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/bin)

add_executable (wavcp wavcp.cpp)
target_link_libraries (wavcp sndfile)

add_executable (wavhist wavhist.cpp)
target_link_libraries (wavhist sndfile)

add_executable (wavquant wavquant.cpp)
target_link_libraries (wavquant sndfile)

add_executable (wavcmp wavcmp.cpp)
target_link_libraries (wavcmp sndfile)

add_executable (wavcb wavcb.cpp)
target_link_libraries (wavcb sndfile)

add_executable (wavcb_2 wavcb_2.cpp)
target_link_libraries (wavcb_2 sndfile)

add_executable (wavvq wavvq.cpp)
target_link_libraries (wavvq sndfile)
