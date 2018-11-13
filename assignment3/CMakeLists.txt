cmake_minimum_required(VERSION 2.8)
project( DisplayImage )

MESSAGE ("\n==> Finding 3rd Party Packages")
find_package( OpenCV REQUIRED )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W" )
SET( CMAKE_C_FLAGS_RELEASE "-O3" )
SET( CMAKE_C_FLAGS_DEBUG "-g3" )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -std=c++11" )
SET( CMAKE_CXX_FLAGS_RELEASE "-O3" )
SET( CMAKE_CXX_FLAGS_DEBUG "-g3" )

SET( BASE_DIR ${CMAKE_SOURCE_DIR} )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/bin )

# Make include directories available for all..
INCLUDE_DIRECTORIES(
  ${OpenCV_INCLUDE_DIR}
)

include_directories(include)
#file(GLOB SOURCES "src/*.cpp")

add_executable( PlayerCamera ${BASE_DIR}/src/PlayerCamera.cpp )
target_link_libraries( PlayerCamera ${OpenCV_LIBS} )

add_executable( PlayerYUV444 ${BASE_DIR}/src//PlayerYUV444.cpp )
target_link_libraries( PlayerYUV444 ${OpenCV_LIBS} )
