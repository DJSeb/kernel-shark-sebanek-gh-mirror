# Like KernelShark's
cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

project(stacklook)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure existing KernelShark
#find_package(KernelShark REQUIRED)
find_package(Qt6Widgets 6.3.0 REQUIRED)
find_package(Qt6 COMPONENTS Network OpenGLWidgets StateMachine REQUIRED)

# include_directories(SYSTEM
#     "/usr/include/qt6"
#     "/usr/include/qt6/QtWidgets"
#     "/usr/include/qt6/QtNetwork"
#     "/usr/include/qt6/QtOpenGLWidgets"
#     "/usr/include/qt6/QtStateMachine"
#     "/usr/include/qt6/QtCore"
#     "/usr/include/qt6/QtGui"
# )

# Ensure a default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

set(SOURCES
    src/stacklook.c
    src/Stacklook.cpp
    # Maybe more later
)

set(KS_FORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../KS_fork")

add_library(stacklook SHARED ${SOURCES})
set_target_properties(stacklook PROPERTIES PREFIX "plugin-")
target_include_directories(stacklook PRIVATE "${KS_FORK_DIR}/src")

set(KS_SHARED_LIBS "${KS_FORK_DIR}/lib")
link_directories(${KS_SHARED_LIBS})
target_link_libraries(stacklook PRIVATE
    kshark
    "/usr/local/lib64/libkshark-plot.so.2.3.1"
    "/usr/local/lib64/libkshark-gui.so.2.3.1"
)
    
# Specify output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)