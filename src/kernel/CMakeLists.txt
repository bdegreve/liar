macro(EVAL)
	foreach(varname ${ARGN})
		message(STATUS "+++ ${varname}: ${${varname}}")
	endforeach()
endmacro()



find_package(LCMS2)
EVAL(LCMS2_FOUND LCMS2_INCLUDE_DIRS LCMS2_LIBRARIES LCMS2_REDISTS LCMS2_DEBUG_REDISTS)
if(LCMS2_FOUND)
	add_definitions(-DLIAR_HAVE_LCMS2_H=1)
	include_directories(${LCMS2_INCLUDE_DIRS})
	install(
		FILES ${LCMS2_REDISTS}
		DESTINATION .
		CONFIGURATIONS RELEASE MINSIZEREL RELWITHDEBINFO
		)
	install(
		FILES ${LCMS2_DEBUG_REDISTS}
		DESTINATION .
		CONFIGURATIONS DEBUG 
		)
endif()



file(GLOB libkernel_SRCS 
	*.h 
	*.inl
	*.cpp
	)	
list_remove_regex(libkernel_SRCS "kernel_init.cpp$")
add_library(libkernel SHARED 
	${libkernel_SRCS}
	)
set_target_properties(
	libkernel
	PROPERTIES 
	COMPILE_FLAGS "${liar_CFLAGS}"
	)
if(APPLE)
	set_target_properties(
		libkernel
		PROPERTIES 
		INSTALL_NAME_DIR "@rpath"
		)
endif()
target_link_libraries(libkernel 
	lass_python
	${LCMS2_LIBRARIES}
	)
install(
	TARGETS libkernel 
	DESTINATION .
	)

add_liar_module_ex(kernel kernel_init.cpp)

