#NOTE: Changed here. (NUMA TV) (2025-06-19)

#[=======================================================================[.rst:
FindHWLOC
-------

Finds the hwloc library.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the :prop_tgt:`IMPORTED` targets:

``hwloc::hwloc``
 Defined if the system has libhwloc.

Result Variables
^^^^^^^^^^^^^^^^

``HWLOC_FOUND``
  True if the system has the libhwloclibrary.
``HWLOC_VERSION``
  The version of the libhwloc library which was found.
``HWLOC_INCLUDE_DIRS``
  Include directories needed to use libhwloc.
``HWLOC_LIBRARIES``
  Libraries needed to link to libhwloc.

Cache Variables
^^^^^^^^^^^^^^^

``HWLOC_INCLUDE_DIR``
  The directory containing ``hwloc.h``.
``HWLOC_LIBRARY``
  The path to the hwloc library.

#]=======================================================================]

find_package(PkgConfig QUIET)
pkg_check_modules(PC_HWLOC QUIET hwloc>=2.11)

set(HWLOC_VERSION     ${PC_HWLOC_VERSION})
set(HWLOC_DEFINITIONS ${PC_HWLOC_CFLAGS_OTHER})

find_path(HWLOC_INCLUDE_DIR NAMES hwloc.h
          HINTS ${PC_HWLOC_INCLUDE_DIRS} ${PC_HWLOC_INCLUDEDIR})

find_library(HWLOC_LIBRARY NAMES hwloc libhwloc
          HINTS ${PC_HWLOC_LIBDIR} ${PC_HWLOC_LIBRARY_DIRS})

mark_as_advanced(HWLOC_INCLUDE_DIR HWLOC_LIBRARY)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(HWLOC DEFAULT_MSG
                                  HWLOC_LIBRARY HWLOC_INCLUDE_DIR)

if(HWLOC_FOUND)
  set(HWLOC_LIBRARIES    ${HWLOC_LIBRARY})
  set(HWLOC_INCLUDE_DIRS ${HWLOC_INCLUDE_DIR})

  if (NOT TARGET hwloc::hwloc)
    add_library(hwloc::hwloc UNKNOWN IMPORTED)
    set_target_properties(hwloc::hwloc
                          PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${HWLOC_INCLUDE_DIRS}"
                            INTERFACE_COMPILE_DEFINITIONS "${HWLOC_DEFINITIONS}"
                            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                            IMPORTED_LOCATION "${HWLOC_LIBRARIES}")
  endif()
endif()
# END of change