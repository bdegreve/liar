if(OpenEXR_ROOT)
    set(_OpenEXR_hints "${OpenEXR_ROOT}" "${OpenEXR_ROOT}/Deploy")
endif()

find_path(OpenEXR_INCLUDE_DIR
    OpenEXR/ImfRgbaFile.h
    HINTS ${_OpenEXR_hints}
    PATH_SUFFIXES include
    )
mark_as_advanced(OpenEXR_INCLUDE_DIR)

if(NOT OpenEXR_ROOT)
    set(_OpenEXR_root)
    if(OpenEXR_INCLUDE_DIR)
        get_filename_component(_OpenEXR_root "${OpenEXR_INCLUDE_DIR}" PATH)
    endif()
    set(OpenEXR_ROOT "${_OpenEXR_root}" CACHE PATH "OpenEXR root directory containing include, lib, bin, ...")
    unset(_OpenEXR_root)
endif()

# Retrieve version from OpenEXR header
unset(OpenEXR_VERSION_STRING)
unset(OpenEXR_VERSION_MAJOR)
unset(OpenEXR_VERSION_MINOR)
unset(OpenEXR_VERSION_PATCH)
unset(_OpenEXR_namespace_version)
if(OpenEXR_INCLUDE_DIR AND EXISTS "${OpenEXR_INCLUDE_DIR}/OpenEXR/OpenEXRConfig.h")
    file(READ "${OpenEXR_INCLUDE_DIR}/OpenEXR/OpenEXRConfig.h" _config)
    string(REGEX REPLACE ".*#define +OPENEXR_VERSION_STRING +\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*" "\\1" OpenEXR_VERSION_STRING "${_config}")
    string(REGEX REPLACE ".*#define +OPENEXR_VERSION_MAJOR +([0-9]+).*" "\\1" OpenEXR_VERSION_MAJOR "${_config}")
    string(REGEX REPLACE ".*#define +OPENEXR_VERSION_MINOR +([0-9]+).*" "\\1" OpenEXR_VERSION_MINOR "${_config}")
    string(REGEX REPLACE ".*#define +OPENEXR_VERSION_PATCH +([0-9]+).*" "\\1" OpenEXR_VERSION_PATCH "${_config}")
    if (OpenEXR_VERSION_MAJOR AND OpenEXR_VERSION_MINOR)
        set(_OpenEXR_namespace_version "${OpenEXR_VERSION_MAJOR}_${OpenEXR_VERSION_MINOR}")
    endif()
endif()

set(_OpenEXR_all_components IlmImf Imath IlmThread Half Iex)
set(_OpenEXR_Imath_dependencies Iex)
set(_OpenEXR_IlmThread_dependencies Iex)
set(_OpenEXR_IlmImf_dependencies Half Imath IlmThread ZLIB)

if(OpenEXR_FIND_COMPONENTS)
    set(_OpenEXR_find_components ${OpenEXR_FIND_COMPONENTS})
else()
    set(_OpenEXR_find_components IlmImf)
endif()

# Recursivily expand components to include all dependencies
set(_OpenEXR_components_modified TRUE)
while(${_OpenEXR_components_modified})
    set(_OpenEXR_components_modified FALSE)
    foreach(_comp ${_OpenEXR_find_components})
        foreach(_OpenEXR_dep ${_OpenEXR_${_comp}_dependencies})
            list(FIND _OpenEXR_find_components "${_OpenEXR_dep}" _OpenEXR_dep_in_components)
            if (_OpenEXR_dep_in_components EQUAL -1)
                list(APPEND _OpenEXR_find_components "${_OpenEXR_dep}")
                set(_OpenEXR_components_modified TRUE)
            endif()
        endforeach()
    endforeach()
endwhile()
unset(_OpenEXR_components_modified)
unset(_OpenEXR_dep_in_components)

set(_OpenEXR_requirements)
foreach(_comp ${_OpenEXR_find_components})
    if(_comp EQUAL "ZLIB")
        list(APPEND _OpenEXR_requirements ZLIB_LIBRARY)
        set(_OpenEXR_find_zlib_arg)
        if(OpenEXR_FIND_QUIETLY)
            list(APPEND _find_zlib_arg QUIET)
        endif()
        find_package(ZLIB ${_find_zlib_arg})
        if(ZLIB_FOUND)
            get_filename_component(_zlib_hint "${ZLIB_LIBRARY}" PATH)
            if (WIN32)
                find_file(ZLIB_REDIST
                    NAMES zlib1.dll
                    HINTS ${_zlib_hint} ${_zlib_hint}/.. ${_zlib_hint}/../bin
                    )
                list(APPEND _redist "${ZLIB_REDIST}")
                list(APPEND _debug_redist "${ZLIB_REDIST}")
            endif()
        endif()
        continue()
    endif()

    find_library(OpenEXR_${_comp}_SHARED_LIBRARY_RELEASE
        NAMES
            "${_comp}-${_OpenEXR_namespace_version}"
            "${_comp}"
        HINTS ${_OpenEXR_hints}
        PATH_SUFFIXES lib lib/Release
    )
    mark_as_advanced(OpenEXR_${_comp}_SHARED_LIBRARY_RELEASE)

    if(WIN32 AND OpenEXR_${_comp}_SHARED_LIBRARY_RELEASE)
        find_file(OpenEXR_${_comp}_REDIST_RELEASE_LIBRARY
            NAMES
                "${_comp}-${_OpenEXR_namespace_version}.dll"
                "${_comp}.dll"
            HINTS ${_OpenEXR_hints}
            PATH_SUFFIXES bin bin/Release
        )
        mark_as_advanced(OpenEXR_${_comp}_REDIST_RELEASE_LIBRARY)
    endif()

    find_library(OpenEXR_${_comp}_SHARED_DEBUG_LIBRARY
        NAMES
            "${_comp}-${_OpenEXR_namespace_version}_d"
            "${_comp}_d"
        HINTS ${_OpenEXR_hints}
        PATH_SUFFIXES lib lib/Debug
    )

    mark_as_advanced(OpenEXR_${_comp}_SHARED_DEBUG_LIBRARY)
    if(WIN32 AND OpenEXR_${_comp}_SHARED_DEBUG_LIBRARY)
        find_file(OpenEXR_${_comp}_REDIST_DEBUG_LIBRARY
            NAMES
                "${_comp}-${_OpenEXR_namespace_version}_d.dll"
                "${_comp}_d.dll"
            HINTS ${_OpenEXR_hints}
            PATH_SUFFIXES bin bin/Debug
        )
        mark_as_advanced(OpenEXR_${_comp}_REDIST_DEBUG_LIBRARY)
    endif()

    find_library(OpenEXR_${_comp}_STATIC_LIBRARY_RELEASE
        NAMES
            "${_comp}-${_OpenEXR_namespace_version}_s"
            "${_comp}_s"
        HINTS ${_OpenEXR_hints}
        PATH_SUFFIXES lib lib/Release
    )
    mark_as_advanced(OpenEXR_${_comp}_STATIC_LIBRARY_RELEASE)

    find_library(OpenEXR_${_comp}_STATIC_LIBRARY_DEBUG
        NAMES
            "${_comp}-${_OpenEXR_namespace_version}_s_d"
            "${_comp}_s_d"
        HINTS ${_OpenEXR_hints}
        PATH_SUFFIXES lib lib/Debug
    )
    mark_as_advanced(OpenEXR_${_comp}_STATIC_LIBRARY_DEBUG)

    if(OpenEXR_${_comp}_SHARED_LIBRARY_RELEASE OR OpenEXR_${_comp}_SHARED_LIBRARY_DEBUG)
        set(_OpenEXR_${_comp}_SHARED TRUE)
        set(_OpenEXR_${_comp}_LIBRARY_RELEASE "${OpenEXR_${_comp}_SHARED_LIBRARY_RELEASE}")
        set(_OpenEXR_${_comp}_LIBRARY_DEBUG "${OpenEXR_${_comp}_SHARED_LIBRARY_DEBUG}")
        set(_OpenEXR_${_comp}_REDIST_RELEASE "${OpenEXR_${_comp}_REDIST_LIBRARY_RELEASE}")
        set(_OpenEXR_${_comp}_REDIST_DEBUG "${OpenEXR_${_comp}_REDIST_LIBRARY_DEBUG}")
    elseif(OpenEXR_${_comp}_STATIC_LIBRARY_RELEASE OR OpenEXR_${_comp}_STATIC_LIBRARY_DEBUG)
        set(_OpenEXR_${_comp}_SHARED FALSE)
        set(_OpenEXR_${_comp}_LIBRARY_RELEASE "${OpenEXR_${_comp}_STATIC_LIBRARY_RELEASE}")
        set(_OpenEXR_${_comp}_LIBRARY_DEBUG "${OpenEXR_${_comp}_STATIC_LIBRARY_DEBUG}")
        unset(_OpenEXR_${_comp}_REDIST_RELEASE)
        unset(_OpenEXR_${_comp}_REDIST_DEBUG)
    else()
        set(_OpenEXR_${_comp}_SHARED FALSE)
        unset(_OpenEXR_${_comp}_LIBRARY_RELEASE)
        unset(_OpenEXR_${_comp}_LIBRARY_DEBUG)
        unset(_OpenEXR_${_comp}_REDIST_RELEASE)
        unset(_OpenEXR_${_comp}_REDIST_DEBUG)
    endif()

    if(_OpenEXR_${_comp}_LIBRARY_RELEASE AND _OpenEXR_${_comp}_LIBRARY_DEBUG)
        set(OpenEXR_${_comp}_LIBRARY 
            optimized "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
            debug "${_OpenEXR_${_comp}_LIBRARY_DEBUG}"
        )
    elseif(_OpenEXR_${_comp}_LIBRARY_RELEASE)
        set(OpenEXR_${_comp}_LIBRARY "${_OpenEXR_${_comp}_LIBRARY_RELEASE}")
        set(OpenEXR_${_comp}_REDIST "${_OpenEXR_${_comp}_REDIST_RELEASE}")
    elseif(_OpenEXR_${_comp}_LIBRARY_DEBUG)
        set(OpenEXR_${_comp}_LIBRARY "${_OpenEXR_${_comp}_LIBRARY_DEBUG}")
        set(OpenEXR_${_comp}_REDIST "${_OpenEXR_${_comp}_REDIST_DEBUG}")
    else()
        unset(OpenEXR_${_comp}_LIBRARY)
        unset(OpenEXR_${_comp}_REDIST)
    endif()

    list(APPEND _OpenEXR_requirements OpenEXR_${_comp}_LIBRARY)
endforeach()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR 
    REQUIRED_VARS OpenEXR_INCLUDE_DIR ${_OpenEXR_requirements}
)


if(OPENEXR_FOUND)
    foreach(_comp ${_OpenEXR_find_components})
        if(NOT TARGET OpenEXR::${_comp})
            if (_OpenEXR_${_comp}_SHARED)
                add_library(OpenEXR::${_comp} SHARED IMPORTED)
                set_property(TARGET OpenEXR::${_comp} APPEND PROPERTY
                    INTERFACE_INCLUDE_DEFINITIONS "OPENEXR_DLL"
                )
            else()
                add_library(OpenEXR::${_comp} STATIC IMPORTED)
            endif()

            set_property(TARGET OpenEXR::${_comp} APPEND PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}"
            )
            set_property(TARGET OpenEXR::${_comp} PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG RELEASE MINSIZEREL RELWITHDEBINFO
            )

            if(WIN32 AND _OpenEXR_${_comp}_SHARED)
                if(_OpenEXR_${_comp}_LIBRARY_DEBUG AND _OpenEXR_${_comp}_LIBRARY_RELEASE)
                    set_target_properties(OpenEXR::${_comp} PROPERTIES
                        IMPORTED_IMPLIB_DEBUG "${_OpenEXR_${_comp}_LIBRARY_DEBUG}"
                        IMPORTED_IMPLIB_RELEASE "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                        IMPORTED_IMPLIB_MINSIZEREL "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                        IMPORTED_IMPLIB_RELWITHDEBINFO "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                        IMPORTED_LOCATION_DEBUG "${OpenEXR_${_comp}_REDIST_DEBUG_LIBRARY}"
                        IMPORTED_LOCATION_RELEASE "${OpenEXR_${_comp}_REDIST_RELEASE_LIBRARY}"
                    )
                else()
                    set_target_properties(OpenEXR::${_comp} PROPERTIES
                        IMPORTED_IMPLIB "${OpenEXR_${_comp}_LIBRARY}"
                        IMPORTED_LOCATION_DEBUG "${OpenEXR_${_comp}_REDIST}"
                    )
                endif()
            else()
                if(_OpenEXR_${_comp}_LIBRARY_DEBUG AND _OpenEXR_${_comp}_LIBRARY_RELEASE)
                    set_target_properties(OpenEXR::${_comp} PROPERTIES
                        IMPORTED_LOCATION_DEBUG "${_OpenEXR_${_comp}_LIBRARY_DEBUG}"
                        IMPORTED_LOCATION_RELEASE "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                        IMPORTED_LOCATION_MINSIZEREL "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                        IMPORTED_LOCATION_RELWITHDEBINFO "${_OpenEXR_${_comp}_LIBRARY_RELEASE}"
                    )
                else()
                    set_target_properties(OpenEXR::${_comp} PROPERTIES
                        IMPORTED_LOCATION "${OpenEXR_${_comp}_LIBRARY}"
                    )
                endif()
            endif()
        endif()
    endforeach()
endif()


# add all dependencies to the different targets
foreach(_comp ${_OpenEXR_find_components})
    if(TARGET "OpenEXR::${_comp}")
        foreach(_OpenEXR_dep ${_OpenEXR_${_comp}_dependencies})
            if(_OpenEXR_dep EQUAL "ZLIB")
                set_property(TARGET "OpenEXR::${_comp}" 
                    APPEND PROPERTY
                    INTERFACE_LINK_LIBRARIES ZLIB::ZLIB
                )
            else()
                set_property(TARGET "OpenEXR::${_comp}" 
                    APPEND PROPERTY
                    INTERFACE_LINK_LIBRARIES "OpenEXR::${_OpenEXR_dep}"
                )
            endif()
        endforeach()
    endif()
endforeach()


#if(OPENEXR_FOUND)
#    set(OpenEXR_INCLUDE_DIRS "${OpenEXR_INCLUDE_DIR}" "${ZLIB_INCLUDE_DIR}")
#
#    # Earlier 1.x versions had problems with #include <OpenEXR/ImfRgbaFile.h> because in ImfRgbaFile.h
#    # they use angled includes for their own headers, like #include <ImfHeader.h>.  That doesn't work
#    # because if the include dir only goes up to OpenEXR, ImfHeader.h can never be found on the include paths.
#    # Try to detect this, and if so, we'll have to add "${OpenEXR_INCLUDE_DIR}/OpenEXR" as well
#    include(CheckIncludeFileCXX)
#    set(CMAKE_REQUIRED_INCLUDES ${OpenEXR_INCLUDE_DIRS})
#    CHECK_INCLUDE_FILE_CXX("OpenEXR/ImfRgbaFile.h" OpenEXR_CAN_INCLUDE_NORMALLY)
#    if(NOT OpenEXR_CAN_INCLUDE_NORMALLY)
#        list(APPEND OpenEXR_INCLUDE_DIRS "${OpenEXR_INCLUDE_DIR}/OpenEXR") 
#        set(CMAKE_REQUIRED_INCLUDES ${OpenEXR_INCLUDE_DIRS})
#        CHECK_INCLUDE_FILE_CXX("OpenEXR/ImfRgbaFile.h" OpenEXR_CAN_INCLUDE_WORKAROUND)
#    endif()
#endif()


foreach(_comp ${_OpenEXR_all_components})
    unset(_OpenEXR_${_comp}_dependencies)
    unset(_OpenEXR_${_comp}_SHARED)
    unset(_OpenEXR_${_comp}_LIBRARY_DEBUG)
    unset(_OpenEXR_${_comp}_LIBRARY_RELEASE)
    unset(_OpenEXR_${_comp}_REDIST_DEBUG)
    unset(_OpenEXR_${_comp}_REDIST_RELEASE)
endforeach()
unset(_OpenEXR_hints)
unset(_OpenEXR_all_components)
unset(_OpenEXR_find_components)
unset(_OpenEXR_requirements)
unset(_OpenEXR_namespace_version)
unset(_comp)