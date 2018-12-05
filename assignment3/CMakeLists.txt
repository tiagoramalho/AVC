cmake_minimum_required(VERSION 2.8)

MESSAGE ("\n==> Finding 3rd Party Packages")
find_package( OpenCV REQUIRED )
find_package( Threads REQUIRED )

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET( BASE_DIR ${CMAKE_SOURCE_DIR} )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/bin )

# Make include directories available for all..
INCLUDE_DIRECTORIES(
  ${OpenCV_INCLUDE_DIR} ${BASE_DIR}/include/
)

#file(GLOB SOURCES "src/*.cpp")

#add_executable( PlayerCamera ${BASE_DIR}/src/PlayerCamera.cpp )
#target_link_libraries( PlayerCamera ${OpenCV_LIBS} )

add_executable( psrb ${BASE_DIR}/src/Player.cpp  ${BASE_DIR}/src/Frame.cpp)
target_link_libraries( psrb ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
