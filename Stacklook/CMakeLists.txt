# Like KernelShark's
cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

project(stacklook)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure existing KernelShark
#find_package(KernelShark REQUIRED)

message("\nINFO: Plugin was developed with KernelShark v2.3.1\n")

set(SL_VERSION_MAJOR 0)
set(SL_VERSION_MINOR 1)
set(SL_VERSION_PATCH 0)
set(SL_VERSION_STR ${SL_VERSION_MAJOR}.${SL_VERSION_MINOR}.${SL_VERSION_PATCH})
message("Current version of Stacklook plugin: ${SL_VERSION_STR}\n")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

set(SOURCES
    src/stacklook.cpp
    # Maybe more later
)

add_library(stacklook SHARED ${SOURCES})
set_target_properties(stacklook PROPERTIES PREFIX "plugin-")
target_include_directories(stacklook PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../../KS_fork/src/")

set(KS_SHARED_LIBS "${CMAKE_CURRENT_BINARY_DIR}/../../KS_fork/lib/")
link_directories(${KS_SHARED_LIBS})
target_link_libraries(stacklook PRIVATE
    kshark
    "/usr/local/lib64/libkshark-plot.so.2.3.1"
    "/usr/local/lib64/libkshark-gui.so.2.3.1"
)
    
# Specify output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)