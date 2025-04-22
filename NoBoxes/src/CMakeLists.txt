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
## KernelShark
### KernelShark files
#### For customisability by the user
if (NOT _KS_INCLUDE_DIR)
  set(_KS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../KS_fork/src")
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

### KernelShark conventions
set(KS_PLUGIN_PREFIX "plugin-")

# Plugin building
## Needed source files
set(SOURCES
    noboxes.h
    noboxes.c
)

## Creating the shared library
add_library(${PLUGIN_NAME} SHARED ${SOURCES})
set_target_properties(${PLUGIN_NAME} PROPERTIES
                      OUTPUT_NAME ${PLUGIN_NAME}
                      PREFIX ${KS_PLUGIN_PREFIX}
                      SUFFIX ".so.${NOBOXES_VERSION}")

## Include KernelShark headers
target_include_directories(${PLUGIN_NAME} PRIVATE ${_KS_INCLUDE_DIR})

## Link KernelShark's shared libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE ${KS_SLIB_CORE})

## Create a symlink to the library for easy access
set(NOBOXES_SYMLINK_NAME "${FINAL_OUTPUT_DIR}/${KS_PLUGIN_PREFIX}${PLUGIN_NAME}.so")
set(NOBOXES_SYMLINK_TARGET "${FINAL_OUTPUT_DIR}/${KS_PLUGIN_PREFIX}${PLUGIN_NAME}.so.${NOBOXES_VERSION}")
add_custom_target("${PLUGIN_NAME}_symlink" ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${NOBOXES_SYMLINK_TARGET} ${NOBOXES_SYMLINK_NAME}
                  BYPRODUCTS "${FINAL_OUTPUT_DIR}/${NOBOXES_SYMLINK_NAME}"
)