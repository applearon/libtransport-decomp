cmake_minimum_required(VERSION 3.10)
project(stream_dock)

# Compiler flags
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -g")
if(DEBUG_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

# output name
set(BINARY_NAME stream_dock)

# Source files
file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Add executable
add_executable(${BINARY_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${BINARY_NAME} -lhidapi-libusb)

find_package(cppzmq)
target_link_libraries(${BINARY_NAME} cppzmq)

# C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
