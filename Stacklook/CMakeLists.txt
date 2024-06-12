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
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
## Ensure existing Qt6
find_package(Qt6Widgets 6.3.0 REQUIRED)
find_package(Qt6 COMPONENTS Network OpenGLWidgets StateMachine REQUIRED)

set(QT6_INCLUDES "/usr/include/qt6")

include_directories(SYSTEM # They are included as system files in KShark
    ${QT6_INCLUDES}
    "${QT6_INCLUDES}/QtWidgets"
    "${QT6_INCLUDES}/QtNetwork"
    "${QT6_INCLUDES}/QtOpenGLWidgets"
    "${QT6_INCLUDES}/QtStateMachine"
    "${QT6_INCLUDES}/QtCore"
    "${QT6_INCLUDES}/QtGui"
)

## KernelShark files
set(KS_PROJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../KS_fork")

set(KS_SHARED_LIBS_DIR "${KS_PROJECT_DIR}/lib")
set(KS_LIB_SUFFIX ".so.2.3.1")
set(KS_LIB_PREFIX "lib")

link_directories(${KS_SHARED_LIBS_DIR})

# Project
set(SOURCES
    src/stacklook.c
    src/Stacklook.cpp
    # Maybe more later
)

add_library(stacklook SHARED ${SOURCES})
set_target_properties(stacklook PROPERTIES PREFIX "plugin-")
target_include_directories(stacklook PRIVATE "${KS_PROJECT_DIR}/src")
target_link_libraries(stacklook PRIVATE
    kshark  kshark-plot${KS_LIB_SUFFIX}  kshark-gui${KS_LIB_SUFFIX}
    #"{KS_PROJECT_DIR}/${KS_LIB_PREFIX}kshark-plot${KS_LIB_SUFFIX}" # Can't use just kshark-plot sadly
    #"{KS_PROJECT_DIR}/${KS_LIB_PREFIX}kshark-gui${KS_LIB_SUFFIX}" # Same here
)