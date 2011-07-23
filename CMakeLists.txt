project(liar)
cmake_minimum_required(VERSION 2.6)
list(APPEND CMAKE_MODULE_PATH "${liar_SOURCE_DIR}/cmake")

set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${liar_BINARY_DIR}/bin")

find_package(Lass REQUIRED)
include_directories(${Lass_INCLUDE_DIRS})
foreach(_config RELEASE DEBUG MINSIZEREL RELWITHDEBINFO)
	get_target_property(_location lass IMPORTED_LOCATION_${_config})
	get_target_property(_location_python lass_python IMPORTED_LOCATION_${_config})
	install(
		FILES "${_location}" "${_location_python}"
		DESTINATION .
		CONFIGURATIONS ${_config}
		)
endforeach()

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

if (Lass_PYTHON_EXECUTABLE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	execute_process(
		COMMAND "${Lass_PYTHON_EXECUTABLE}" -c "from distutils import sysconfig; print sysconfig.get_python_lib(True)" 
		OUTPUT_VARIABLE _python_site_packages
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set (CMAKE_INSTALL_PREFIX "${_python_site_packages}/liar" CACHE PATH "liar install prefix" FORCE)
endif ()

if (MSVC_IDE)
	option(BUILD_USING_MULTIPLE_PROCESSES ON)
else()
	set(BUILD_USING_MULTIPLE_PROCESSES OFF)
endif()

set(liar_CFLAGS)
if(MSVC)
	set (liar_CFLAGS "${liar_CFLAGS} /EHsc")
	if(NOT CMAKE_CL_64)
		set (liar_CFLAGS "${liar_CFLAGS} /arch:SSE2 /fp:fast")
	endif()
	if(BUILD_USING_MULTIPLE_PROCESSES)
		set (liar_CFLAGS "${liar_CFLAGS} /MP")
	endif()
else()
	set (liar_CFLAGS "${liar_CFLAGS} -fno-strict-aliasing -Wall -Wextra -Wformat=2 -Winit-self -Wno-unknown-pragmas")
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

macro(add_liar_module_ex name)
	set(_srcs)
	set(_libs)
	set(_destination ".")
	set(_list "_srcs")
	foreach(_arg ${ARGN})
		if("${_arg}" STREQUAL "SRCS")
			set(_list "_srcs")
		elseif("${_arg}" STREQUAL "LIBS")
			set(_list "_libs")
		elseif("${_arg}" STREQUAL "DESTINATION")
			set(_list "_destination")
		else()
			if ("${_list}" STREQUAL "_destination")
				set(${_list} "${_arg}")
			else()
				list(APPEND ${_list} "${_arg}")
			endif()
		endif()
	endforeach()
	add_library(${name} MODULE 
		${_srcs}
		)
	target_link_libraries(${name} 
		libkernel
		${_libs}
		)
	set_target_properties(
		${name}
		PROPERTIES 
		COMPILE_FLAGS "${liar_CFLAGS}"
		)
	if (WIN32)
		set_target_properties(
			${name}
			PROPERTIES 
			SUFFIX ".pyd"
			)
	else()
		set_target_properties(
			${name}
			PROPERTIES 
			PREFIX ""
			)	
	endif()
	install(
		TARGETS ${name}
		DESTINATION "${_destination}"
		)
endmacro()

macro(add_liar_module name)
	file(GLOB ${name}_SRCS *.h *.inl *.cpp)
	set(_option)
	foreach(_arg ${ARGN})
		if("${_arg}" STREQUAL "IGNORE")
			set(_option "IGNORE")
		else()
			if("${_option}" STREQUAL "IGNORE")
				list_remove_regex(${name}_SRCS ${_arg})
			else()
				message(SEND_ERROR "unexpected argument")
			endif()
		endif()
	endforeach()
	add_liar_module_ex(${name} 
		SRCS ${${name}_SRCS}
		LIBS ${${name}_LIBS}
	)
endmacro()

add_subdirectory(src)
add_subdirectory(doc)

set(BUILD_TESTING "ON" CACHE BOOL "build test")
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
