cmake_minimum_required(VERSION 3.0)
project(liar VERSION 0.2.0)
list(APPEND CMAKE_MODULE_PATH "${liar_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/lib")

macro(eval)
	foreach(arg ${ARGN})
		message(STATUS "${arg}: ${${arg}}")
	endforeach()
endmacro()

# if CMAKE_INSTALL_PREFIX is initialized by default, empty it, so it can be set later
# when we figure out what python to use.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set (CMAKE_INSTALL_PREFIX "" CACHE PATH "liar install prefix" FORCE)
endif ()

find_package(Lass 1.9 REQUIRED)

if (CMAKE_CONFIGURATION_TYPES)
	set(_configs ${CMAKE_CONFIGURATION_TYPES})
else()
	set(_configs "${CMAKE_BUILD_TYPE}")
endif()
foreach(_config ${_configs})
	string(TOUPPER "${_config}" _CONFIG)
	get_target_property(_location Lass::lass IMPORTED_LOCATION_${_CONFIG})
	get_target_property(_location_python Lass::lass_python IMPORTED_LOCATION_${_CONFIG})
	if (NOT _location OR NOT _location_python)
		#message(SEND_ERROR "can't find lass libraries for ${_config} build")
	endif()
	install(
		FILES "${_location}" "${_location_python}"
		DESTINATION .
		CONFIGURATIONS ${_config}
		)
endforeach()
if (Python_LIBRARY_DEBUG)
	set(CMAKE_DEBUG_POSTFIX _d)
endif()

set (CMAKE_INSTALL_DEBUG_LIBRARIES 1)
set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP 1)
include(InstallRequiredSystemLibraries)
if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
	install(
		PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
		DESTINATION .
		)
endif()
if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_DEBUG)
	install(
		PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_DEBUG}
		DESTINATION .
		)
endif()

if (Python_EXECUTABLE AND NOT CMAKE_INSTALL_PREFIX)
	execute_process(
		COMMAND "${Python_EXECUTABLE}" -c "from distutils import sysconfig; print (sysconfig.get_python_lib(True))"
		OUTPUT_VARIABLE _python_site_packages
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set (CMAKE_INSTALL_PREFIX "${_python_site_packages}/liar" CACHE PATH "liar install prefix" FORCE)
endif ()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

if (MSVC)
	option(BUILD_USING_PRECOMPILED_HEADERS "Build using precompiled headers" ON)
else()
	set(BUILD_USING_PRECOMPILED_HEADERS OFF)
endif()

if (MSVC_IDE AND NOT MSVC_VERSION LESS 1400)
	if (MSVC_VERSION LESS 1500)
		set(_default_mp OFF)
	else()
		set(_default_mp ON)
	endif()
	option(BUILD_USING_MULTIPLE_PROCESSES "Enable parallel compilation within one project (/MP)" ${_default_mp})
else()
	set(BUILD_USING_MULTIPLE_PROCESSES OFF)
endif ()

set(liar_CFLAGS)
set(liar_LDFLAGS)
if(MSVC)
	set (liar_CFLAGS "/EHsc")
	if(NOT CMAKE_CL_64)
		set (liar_CFLAGS "${liar_CFLAGS} /arch:SSE2 /fp:fast")
	endif()
	if(BUILD_USING_MULTIPLE_PROCESSES)
		set (liar_CFLAGS "${liar_CFLAGS} /MP")
	endif()
else()
	set (liar_CFLAGS "-std=c++11 -fno-strict-aliasing -Wall -Wextra -Wformat=2 -Winit-self -Wconversion -Wno-unknown-pragmas -Wno-deprecated-declarations")
	if(APPLE)
		set (liar_LDFLAGS "-Wl,-rpath,@loader_path/.")
	endif()
endif()

macro(list_remove_regex listvar regex)
	set(_to_be_removed)
	foreach(_path ${${listvar}})
		if("${_path}" MATCHES "${regex}")
			list(APPEND _to_be_removed "${_path}")
		endif()
	endforeach()
	list(REMOVE_ITEM ${listvar} ${_to_be_removed})
endmacro()

macro(_mark_target_as_python_module name)
endmacro()

function(set_precompiled_header target hdrfile srcpath)
    if(NOT BUILD_USING_PRECOMPILED_HEADERS)
        return()
    endif()

    if(MSVC_IDE)
        set(pchpath "${CMAKE_CURRENT_BINARY_DIR}/${target}.dir/${CMAKE_CFG_INTDIR}/${hdrfile}.pch")
    else()
        set(pchpath "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/${hdrfile}.pch")
    endif()
    if(MSVC)
        set_property(
            TARGET "${target}"
            APPEND_STRING
            PROPERTY COMPILE_FLAGS " /Fp\"${pchpath}\" /Yu\"${hdrfile}\""
        )
        set_property(
            SOURCE "${srcpath}"
            APPEND_STRING
            PROPERTY COMPILE_FLAGS " /Yc\"${hdrfile}\""
        )
        if (NOT MSVC_IDE)
            get_target_property(sources "${target}" SOURCES)
            list(REMOVE_ITEM sources "${srcpath}")
            set_property(
                SOURCE ${sources}
                APPEND
                PROPERTY OBJECT_DEPENDS "${pchpath}"
            )
        endif()
    else()
        #add_custom_command(
        #    OUTPUT ${pchpath}
        #    DEPENDS ${hdrfile}
        #    COMMAND ...)
        #set_source_files_properties(
        #    ${test_SRCS}
        #    PROPERTIES
        #    OBJECT_DEPENDS ${_pchpath})
    endif()
endfunction()


include(CMakeParseArguments)

function(add_liar_module_ex name)
	message(STATUS "add_liar_module ${name}")

	set(_prefix)
	set(_options)
	set(_one_value_keywords "DESTINATION")
	set(_multi_value_keywords "SRCS" "LIBS")
	cmake_parse_arguments(
		"${_prefix}" "${_options}" "${_one_value_keywords}" "${_multi_value_keywords}"
		"SRCS" ${ARGN})
	if(NOT _DESTINATION)
		set(_DESTINATION ".")
	endif()

	add_library("${name}" MODULE
		${_SRCS}
		)
	target_link_libraries("${name}"
		libkernel
		${_LIBS}
		)
	set_target_properties(
		"${name}"
		PROPERTIES
		COMPILE_FLAGS "${liar_CFLAGS}"
		LINK_FLAGS "${liar_LDFLAGS}"
		)
	if (WIN32)
		set_target_properties(
			"${name}"
			PROPERTIES
			SUFFIX ".pyd"
			)
	else()
		set_target_properties(
			"${name}"
			PROPERTIES
			PREFIX ""
			)
	endif()
	install(
		TARGETS "${name}"
		DESTINATION "${_DESTINATION}"
		)
endfunction()

function(add_liar_module name)
	file(GLOB _srcs *.h *.inl *.cpp)
	set(_option)
	foreach(_arg ${ARGN})
		if("${_arg}" STREQUAL "IGNORE")
			set(_option "IGNORE")
		else()
			if("${_option}" STREQUAL "IGNORE")
				list_remove_regex(_srcs ${_arg})
			else()
				message(SEND_ERROR "unexpected argument")
			endif()
		endif()
	endforeach()
	add_liar_module_ex("${name}"
		SRCS ${${name}_SRCS} ${_srcs}
		LIBS ${${name}_LIBS}
	)
	set_precompiled_header("${name}" "${name}_common.h" "${name}_common.cpp")
endfunction()

set(SPECTRAL_MODE "RGB" CACHE STRING "Computational mode for Spectral values")
set(SPECTRAL_NUM_BANDS 10 CACHE STRING "Number of bands")
set(SPECTRAL_MIN_WAVELENGTH 380 CACHE STRING "Lowerbound")
set(SPECTRAL_MAX_WAVELENGTH 720 CACHE STRING "Upperbound")
set_property(CACHE SPECTRAL_MODE PROPERTY STRINGS XYZ RGB Banded Single)
unset(LIAR_SPECTRAL_MODE_XYZ)
unset(LIAR_SPECTRAL_MODE_RGB)
unset(LIAR_SPECTRAL_MODE_BANDED)
unset(LIAR_SPECTRAL_MODE_SINGLE)
unset(LIAR_SPECTRAL_MIN_WAVELENGTH)
unset(LIAR_SPECTRAL_MAX_WAVELENGTH)
if (SPECTRAL_MODE STREQUAL "XYZ")
	set(LIAR_SPECTRAL_MODE_XYZ ON)
elseif (SPECTRAL_MODE STREQUAL "RGB")
	set(LIAR_SPECTRAL_MODE_RGB ON)
elseif(SPECTRAL_MODE STREQUAL "Banded")
	set(LIAR_SPECTRAL_MODE_BANDED "${SPECTRAL_NUM_BANDS}")
	set(LIAR_SPECTRAL_MIN_WAVELENGTH "${SPECTRAL_MIN_WAVELENGTH}")
	set(LIAR_SPECTRAL_MAX_WAVELENGTH "${SPECTRAL_MAX_WAVELENGTH}")
elseif(SPECTRAL_MODE STREQUAL "Single")
	set(LIAR_SPECTRAL_MODE_SINGLE ON)
else()
	message(FATAL_ERROR "Invalid value for SPECTRAL_MODE: ${SPECTRAL_MODE}")
endif()

configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/src/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.h"
	)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)

set(BUILD_TESTING "ON" CACHE BOOL "build test")
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
