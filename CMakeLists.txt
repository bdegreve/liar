project(liar)
cmake_minimum_required(VERSION 2.6)

find_package(lass REQUIRED)
include_directories(${lass_INCLUDE_DIRS})

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
	target_link_libraries(${name} 
		${${name}_LIBS}
		)
	install(
		TARGETS ${name}
		DESTINATION .
		)
endmacro()

add_subdirectory(src)