macro(EVAL)
	foreach(varname ${ARGN})
		message(STATUS "+++ ${varname}: ${${varname}}")
	endforeach()
endmacro()


option(LCMS2_ENABLE "Use LCMS2 as color conversion engine" "OFF")
if (LCMS2_ENABLE)
	find_package(LCMS2)
	if (LCMS2_FOUND)
		add_definitions(-DLIAR_HAVE_LCMS2_H=1)
		include_directories(${LCMS2_INCLUDE_DIRS})
		install(
			FILES ${LCMS2_REDISTS}
			DESTINATION liar
			CONFIGURATIONS RELEASE MINSIZEREL RELWITHDEBINFO
			)
		install(
			FILES ${LCMS2_DEBUG_REDISTS}
			DESTINATION liar
			CONFIGURATIONS DEBUG
			)
	else()
		message(SEND_ERROR "LCMS2 not found. Fix or disable LCMS2.")
	endif()
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
target_compile_definitions(libkernel
	PUBLIC
		"$<$<CONFIG:Debug>:LIAR_DEBUG>"
		"$<$<CONFIG:RelWithDebInfo>:LIAR_DEBUG>"
)
target_link_libraries(libkernel
	Lass::lass_python
	${LCMS2_LIBRARIES}
	)
set_precompiled_header(libkernel kernel_common.h kernel_common.cpp)
install(
	TARGETS libkernel
	DESTINATION liar
	)

add_liar_module_ex(kernel kernel_init.cpp)
