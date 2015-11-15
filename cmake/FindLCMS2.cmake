if(LCMS2_INCLUDE_DIR)
  set(LCMS2_FIND_QUIETLY TRUE)
endif()

set(_hints "${LCMS2_DIR}")

find_path(LCMS2_INCLUDE_DIR
    lcms2.h
    HINTS ${_hints}
    PATH_SUFFIXES include
    )

if(NOT LCMS2_DIR)
    set(_root)
    if(LCMS2_INCLUDE_DIR)
        get_filename_component(_root "${LCMS2_INCLUDE_DIR}" PATH)
    endif()
    set(LCMS2_DIR "${_root}" CACHE PATH "LitleCMS2 root directory containing include, lib, bin, ..." FORCE)
endif()

find_library(LCMS2_LIBRARY
    NAMES lcms2
    HINTS ${_hints}
    PATH_SUFFIXES bin
    )
find_library(LCMS2_DEBUG_LIBRARY
    NAMES lcms2d
    HINTS ${_hints}
    PATH_SUFFIXES bin
    )

if (WIN32)
    find_file(LCMS2_REDIST
        NAMES lcms2.dll
        HINTS ${_hints}
        PATH_SUFFIXES bin
        )
    find_file(LCMS2_DEBUG_REDIST
        NAMES lcms2d.dll
        HINTS ${_hints}
        PATH_SUFFIXES bin
        )
endif ()

mark_as_advanced(
    LCMS2_INCLUDE_DIR
    LCMS2_LIBRARY
    LCMS2_DEBUG_LIBRARY
    LCMS2_REDIST
    LCMS2_DEBUG_REDIST
    )

# add the found goodies to some lists.
set(LCMS2_INCLUDE_DIRS ${LCMS2_INCLUDE_DIR})
if(LCMS2_LIBRARY)
    if (LCMS2_DEBUG_LIBRARY)
        list(APPEND LCMS2_LIBRARIES optimized "${LCMS2_LIBRARY}")
    else()
        list(APPEND LCMS2_LIBRARIES "${LCMS2_LIBRARY}")
    endif()
endif()
if(LCMS2_DEBUG_LIBRARY)
    list(APPEND LCMS2_LIBRARIES debug "${LCMS2_DEBUG_LIBRARY}")
endif()
if(LCMS2_REDIST)
    list(APPEND LCMS2_REDISTS "${LCMS2_REDIST}")
endif()
if(LCMS2_DEBUG_REDIST)
    list(APPEND LCMS2_DEBUG_REDISTS "${LCMS2_DEBUG_REDIST}")
endif()


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LCMS2 DEFAULT_MSG LCMS2_LIBRARIES LCMS2_INCLUDE_DIRS)
