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

add_subdirectory(src)