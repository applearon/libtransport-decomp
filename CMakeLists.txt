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


find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

target_include_directories(${BINARY_NAME} PRIVATE ${DBUS_INCLUDE_DIRS})
target_link_libraries(${BINARY_NAME} PRIVATE ${DBUS_LIBRARIES})
add_definitions(${DBUS_CFLAGS_OTHER}) # optional but recommended

# Link libraries
target_link_libraries(${BINARY_NAME} PRIVATE -lhidapi-libusb)


# C++ standard
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
