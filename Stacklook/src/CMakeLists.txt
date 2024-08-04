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

# Specify output directory
set(FINAL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
make_directory(${FINAL_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${FINAL_OUTPUT_DIR})

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
## Same as KernelShark
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Wextra -pthread -fPIC -fno-common")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread -fPIC -fno-common")

# Dependencies
## Ensure existing Qt6 and necessary components
find_package(Qt6Widgets 6.3.0 REQUIRED)
find_package(Qt6 COMPONENTS Network OpenGLWidgets StateMachine REQUIRED)

## For customisability by the user
if (NOT _QT6_INCLUDE_DIR)
    set(_QT6_INCLUDE_DIR "/usr/include/qt6")
endif()

set(QT6_ALL_INCLUDES
    ${_QT6_INCLUDE_DIR}
    "${_QT6_INCLUDE_DIR}/QtWidgets"
    "${_QT6_INCLUDE_DIR}/QtNetwork"
    "${_QT6_INCLUDE_DIR}/QtOpenGLWidgets"
    "${_QT6_INCLUDE_DIR}/QtStateMachine"
    "${_QT6_INCLUDE_DIR}/QtCore"
    "${_QT6_INCLUDE_DIR}/QtGui"
)

## KernelShark
### KernelShark files
#### For customisability by the user
if (NOT _KS_INCLUDE_DIR)
    set(_KS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../kernelshark/src")
endif()

#### For customisability by the user
if (NOT _KS_SHARED_LIBS_DIR)
    set(_KS_SHARED_LIBS_DIR "/usr/local/lib64")
endif()

# The below specifications of names, suffixes and prefixes seems to
# be necessary in some contexts, where CMake cannot find the SOs if
# they end in version number, hence we search for explicit files.
set(KS_LIB_SUFFIX ".so.${KS_VERSION}")
set(KS_LIB_PREFIX "lib")

set(KS_SLIB_CORE "${_KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark${KS_LIB_SUFFIX}")
set(KS_SLIB_PLOT "${_KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark-plot${KS_LIB_SUFFIX}")
set(KS_SLIB_GUI "${_KS_SHARED_LIBS_DIR}/${KS_LIB_PREFIX}kshark-gui${KS_LIB_SUFFIX}")

### KernelShark conventions
set(KS_PLUGIN_PREFIX "plugin-")

# Plugin building
## Needed source files
set(SOURCES
    stacklook.h
    SlButton.hpp
    SlDetailedView.hpp
    stacklook.c
    SlButton.cpp
    SlDetailedView.cpp
    Stacklook.cpp
)

## Creating the shared library
add_library(stacklook SHARED ${SOURCES})
set_target_properties(stacklook PROPERTIES
                      PREFIX ${KS_PLUGIN_PREFIX}
                      SUFFIX ".so.${SL_VERSION}")

## Include KernelShark headers
target_include_directories(stacklook PRIVATE ${_KS_INCLUDE_DIR})
## Below are included as system header files in KShark, hence SYSTEM here
target_include_directories(stacklook SYSTEM PRIVATE ${QT6_ALL_INCLUDES})

## Link KernelShark's shared libraries
target_link_libraries(stacklook PRIVATE
    ${KS_SLIB_CORE}  ${KS_SLIB_PLOT}  ${KS_SLIB_GUI}
)

set(SL_SYMLINK_NAME "${FINAL_OUTPUT_DIR}/${KS_PLUGIN_PREFIX}stacklook.so")
set(SL_SYMLINK_TARGET "${FINAL_OUTPUT_DIR}/${KS_PLUGIN_PREFIX}stacklook.so.${SL_VERSION}")
add_custom_target(stacklook_symlink ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${SL_SYMLINK_TARGET} ${SL_SYMLINK_NAME})