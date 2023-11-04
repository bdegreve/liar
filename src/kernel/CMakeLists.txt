macro(EVAL)
	foreach(varname ${ARGN})
		message(STATUS "+++ ${varname}: ${${varname}}")
	endforeach()
endmacro()

file(GLOB libkernel_SRCS
	*.h
	*.inl
	*.cpp
	)
list_remove_regex(libkernel_SRCS "kernel_init.cpp$")
add_library(libkernel SHARED
	${libkernel_SRCS}
	)
if(WIN32)
	# pass
elseif(APPLE)
	set_target_properties(
		libkernel
		PROPERTIES
		INSTALL_NAME_DIR "@rpath"
		)
else()
	set_target_properties(
		libkernel
		PROPERTIES
			INSTALL_RPATH "$ORIGIN"
		)
endif()
if(NOT DEFINED liar_DEBUG)
	# enable it automatically for debug and relwithdebinfo builds
	target_compile_definitions(libkernel
		PUBLIC
			"$<$<CONFIG:Debug>:LIAR_DEBUG>"
			"$<$<CONFIG:RelWithDebInfo>:LIAR_DEBUG>"
	)
elseif(liar_DEBUG)
	target_compile_definitions(libkernel
		PUBLIC
			LIAR_DEBUG
	)
endif()
target_link_libraries(libkernel
	PUBLIC
		Lass::lass_python
	)
target_precompile_headers(libkernel PRIVATE kernel_common.h)
if(WIN32)
	add_custom_command(TARGET libkernel POST_BUILD
		COMMAND "${CMAKE_COMMAND}" ARGS -E copy_if_different "$<TARGET_RUNTIME_DLLS:libkernel>" "$<TARGET_FILE_DIR:libkernel>"
		COMMAND_EXPAND_LISTS
	)
endif()
install(
	TARGETS libkernel
	DESTINATION liar
	)

add_liar_module_ex(kernel kernel_init.cpp)

option(LCMS2_ENABLE "Use Little CMS as color conversion engine" "OFF")
if (LCMS2_ENABLE)
	find_package(lcms REQUIRED)
	target_link_libraries(libkernel
		PRIVATE lcms::lcms
	)
	target_compile_definitions(libkernel
		PUBLIC LIAR_HAVE_LCMS2_H=1
	)
endif()
