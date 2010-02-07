find_path(OpenEXR_INCLUDE_DIR 
	ImfRgbaFile.h
	HINTS ${OpenEXR_DIR}
	PATH_SUFFIXES include
	)

if(NOT OpenEXR_DIR)
	set(_root)
	if(OpenEXR_INCLUDE_DIR)
		get_filename_component(_root "${OpenEXR_INCLUDE_DIR}" PATH)
	endif()
	set(OpenEXR_DIR "${_root}" CACHE PATH "OpenEXR root directory containing include, lib, bin, ...")
endif()

if(OpenEXR_FIND_COMPONENTS)
	set(_components ${OpenEXR_FIND_COMPONENTS})
else()
	set(_components Half Iex IlmCtl IlmCtlMath IlmCtlSimd IlmImf IlmThread Imath)
endif()

set(_definitions)
if (WIN32)
	list(APPEND _definitions -DOPENEXR_DLL)
endif()

set(_libraries)
set(_redist)
set(_redist_debug)
foreach(_comp ${_components})
	if(MSVC_IDE)
		# we're looking for both the release and debug libraries
		find_library(OpenEXR_${_comp}_LIBRARY
			NAMES ${_comp}
			HINTS ${OpenEXR_DIR}
			PATH_SUFFIXES lib lib/Release
			)
		find_library(OpenEXR_${_comp}_DEBUG_LIBRARY
			NAMES ${_comp}
			HINTS ${OpenEXR_DIR}
			PATH_SUFFIXES lib lib/Debug
			)
		find_file(OpenEXR_${_comp}_REDIST
			NAMES ${_comp}.dll
			HINTS ${OpenEXR_DIR}
			PATH_SUFFIXES bin bin/Release
			)
		find_file(OpenEXR_${_comp}_DEBUG_REDIST
			NAMES ${_comp}.dll
			HINTS ${OpenEXR_DIR}
			PATH_SUFFIXES bin bin/Debug
			)
		if(OpenEXR_${_comp}_LIBRARY)
			if (OpenEXR_${_comp}_DEBUG_LIBRARY)
				list(APPEND _libraries optimized "${OpenEXR_${_comp}_LIBRARY}" debug "${OpenEXR_${_comp}_DEBUG_LIBRARY}")
			else()
				list(APPEND _libraries "${OpenEXR_${_comp}_LIBRARY}")
			endif()
		endif()
		if(OpenEXR_${_comp}_REDIST)
			list(APPEND _redist "${OpenEXR_${_comp}_REDIST}")
		endif()
		if(OpenEXR_${_comp}_DEBUG_REDIST)
			list(APPEND _debug_redist "${OpenEXR_${_comp}_DEBUG_REDIST}")
		endif()
	else()
		set(_subdir Release)
		string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
		if("${_build_type}" STREQUAL "debug")
			set(_subdir Debug)
		endif()
		find_library(OpenEXR_${_comp}_LIBRARY
			NAMES ${_comp}
			HINTS ${OpenEXR_ROOT}
			PATH_SUFFIXES lib "lib/${_subdir}"
			)
		if(OpenEXR_${_comp}_LIBRARY)
			list(APPEND _libraries "${OpenEXR_${_comp}_LIBRARY}")
		endif()
	endif()
endforeach()

set(_find_zlib_arg)
if(OpenEXR_FIND_QUIETLY)
	list(APPEND _find_zlib_arg QUIET)
endif()
find_package(ZLIB ${_find_zlib_arg})

include(FindPackageHandleStandardArgs)
set(_requirements)
foreach(_comp ${_components})
	list(APPEND _requirements OpenEXR_${_comp}_LIBRARY)
endforeach()
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenEXR DEFAULT_MSG OpenEXR_INCLUDE_DIR ${_requirements} ZLIB_INCLUDE_DIRS ZLIB_LIBRARIES)

if(OPENEXR_FOUND)
	set(OpenEXR_INCLUDE_DIRS "${OpenEXR_INCLUDE_DIR}" "${ZLIB_INCLUDE_DIRS}")
	set(OpenEXR_LIBRARIES "${_libraries}" "${ZLIB_LIBRARIES}")
	set(OpenEXR_DEFINITIONS "${_definitions}")
	set(OpenEXR_REDIST "${_redist}")
	set(OpenEXR_DEBUG_REDIST "${_debug_redist}")
endif()
