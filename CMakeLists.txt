cmake_minimum_required(VERSION 3.16)
project(liar VERSION 0.2.0)
list(APPEND CMAKE_MODULE_PATH "${liar_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/lib")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

find_package(Lass 1.11 REQUIRED)

install(IMPORTED_RUNTIME_ARTIFACTS Lass::lass Lass::lass_python
	DESTINATION liar
)


if (Python_LIBRARY_DEBUG)
	set(CMAKE_DEBUG_POSTFIX _d)
endif()

set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP 1)
include(InstallRequiredSystemLibraries)
install(
	PROGRAMS "$<$<NOT:$<CONFIG:Debug>>:${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}>"
	DESTINATION liar
)

if (Python_EXECUTABLE AND NOT CMAKE_INSTALL_PREFIX)
	execute_process(
		COMMAND "${Python_EXECUTABLE}" -c "from distutils import sysconfig; print (sysconfig.get_python_lib(True))"
		OUTPUT_VARIABLE _python_site_packages
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set (CMAKE_INSTALL_PREFIX "${_python_site_packages}" CACHE PATH "liar install prefix" FORCE)
endif ()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

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

if(MSVC)
	add_compile_options(
		"/EHsc"
		"/arch:AVX2"
		"/fp:fast"
		"$<$<CONFIG:Release>:/GL>"
		"$<$<CONFIG:Release>:/Zi>"
	)
	add_link_options(
		"$<$<CONFIG:Release>:/LTCG>"
		"$<$<CONFIG:Release>:/DEBUG>"
		"$<$<CONFIG:Release>:/OPT:REF>"
		"$<$<CONFIG:Release>:/OPT:ICF>"
	)
	if(BUILD_USING_MULTIPLE_PROCESSES)
		add_compile_options("/MP")
	endif()
else()
	add_compile_options(
		"-fno-strict-aliasing"
		"-march=haswell"
		"$<$<CONFIG:Release>:-g>"
		"-Wall"
		"-Wextra"
		"-Wformat=2"
		"-Winit-self"
		"-Wconversion"
		"-Wno-unknown-pragmas"
		"-Wno-deprecated-declarations"
	)
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
		set(_DESTINATION "liar")
	endif()

	Python_add_library ("${name}" ${_SRCS} WITH_SOABI)
	target_link_libraries("${name}"
		PRIVATE libkernel
		PRIVATE ${_LIBS}
		)
	set_target_properties(
		"${name}"
		PROPERTIES
			LINK_FLAGS "${liar_LDFLAGS}"
		)
	if (NOT WIN32)
		set_target_properties(
			"${name}"
			PROPERTIES
				INSTALL_RPATH "$ORIGIN"
			)
		if(APPLE)
			target_link_options("${name}"
				PRIVATE "-Wl,-rpath,@loader_path/."
			)
		endif()
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
	target_precompile_headers("${name}" PRIVATE "${name}_common.h")
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

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)

set(BUILD_TESTING "ON" CACHE BOOL "build test")
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
	set_directory_properties(
		PROPERTIES
			VS_STARTUP_PROJECT test_driver
	)
endif()
