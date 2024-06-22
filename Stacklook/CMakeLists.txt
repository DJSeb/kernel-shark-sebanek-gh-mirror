# A little bit of a commentary on this file.
# If the project source files were simply added
# to the KernelShark/src/plugins directory and
# the plugins CMakeLists file were slightly changed,
# then this file would not be necessary, as KernelShark
# already provides the build instructions.
#
# This will still link all the necessary libraries, and
# include necessary files, but a bit more verbosely due
# to the location of the plugin project :)

# CMake config

# Like KernelShark's
cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

project(stacklook)

# Ensure a default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebug)
endif (NOT CMAKE_BUILD_TYPE)
    
# Specify output directory
set(FINAL_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/bin")
make_directory(${FINAL_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR})

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
## Ensure existing Qt6
find_package(Qt6Widgets 6.3.0 REQUIRED)
find_package(Qt6 COMPONENTS Network OpenGLWidgets StateMachine REQUIRED)

set(QT6_INCLUDES "/usr/include/qt6")
set(QT6_ALL_INCLUDES
    ${QT6_INCLUDES}
    "${QT6_INCLUDES}/QtWidgets"
    "${QT6_INCLUDES}/QtNetwork"
    "${QT6_INCLUDES}/QtOpenGLWidgets"
    "${QT6_INCLUDES}/QtStateMachine"
    "${QT6_INCLUDES}/QtCore"
    "${QT6_INCLUDES}/QtGui"
)

## KernelShark
### KernelShark meta
set(KS_VERSION "2.3.1")

### KernelShark files
set(KS_PROJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../KS_fork")
set(KS_INCLUDES "${KS_PROJECT_DIR}/src")

set(KS_SHARED_LIBS_DIR "/usr/local/lib64")
set(KS_LIB_SUFFIX ".so.${KS_VERSION}")
set(KS_LIB_PREFIX "lib")

set(KS_SLIB_CORE "${KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark${KS_LIB_SUFFIX}")
set(KS_SLIB_PLOT "${KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark-plot${KS_LIB_SUFFIX}")
set(KS_SLIB_GUI "${KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark-gui${KS_LIB_SUFFIX}")

### KernelShark conventions
set(KS_PLUGIN_PREFIX "plugin-")

# Project
set(SOURCES
    src/stacklook.h
    src/Stacklook.hpp
    src/stacklook.c
    src/Stacklook.cpp
)

add_library(stacklook SHARED ${SOURCES})
set_target_properties(stacklook PROPERTIES PREFIX ${KS_PLUGIN_PREFIX})

target_include_directories(stacklook PRIVATE ${KS_INCLUDES})

# Below are included as system header files in KShark, hence SYSTEM here
target_include_directories(stacklook SYSTEM PRIVATE ${QT6_ALL_INCLUDES})
target_link_libraries(stacklook PRIVATE
    ${KS_SLIB_CORE}  ${KS_SLIB_PLOT}  ${KS_SLIB_GUI}
)