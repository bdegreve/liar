project(liar)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_DEBUG_POSTFIX _d)

find_package(lass REQUIRED)
include_directories(${lass_INCLUDE_DIRS})
foreach(_config RELEASE DEBUG MINSIZEREL RELWITHDEBINFO)
	get_target_property(_location lass IMPORTED_LOCATION_${_config})
	install(
		FILES "${_location}"
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

macro(list_remove_regex listvar regex)
	set(_to_be_removed)
	foreach(_path ${${listvar}})
		if("${_path}" MATCHES "${regex}")
			list(APPEND _to_be_removed "${_path}")
		endif()
	endforeach()
	list(REMOVE_ITEM ${listvar} ${_to_be_removed})
endmacro()

macro(add_liar_module name)
	file(GLOB ${name}_HDRS *.h *.inl)
	file(GLOB ${name}_SRCS *.cpp)
	set(${name}_LIBS kernelshared)
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
	add_library(${name} MODULE 
		${${name}_HDRS} 
		${${name}_SRCS}
		)
	if (WIN32)
		set_target_properties(
			${name}
			PROPERTIES 
			SUFFIX ".pyd"
			)
	endif()
	target_link_libraries(${name} 
		${${name}_LIBS}
		)
	install(
		TARGETS ${name}
		DESTINATION .
		)
endmacro()

add_subdirectory(src)

set(BUILD_TESTING "ON" CACHE BOOL "build test")
if(BUILD_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()