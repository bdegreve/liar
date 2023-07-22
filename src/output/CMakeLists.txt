set (_libraries)
set (_srcs)

add_liar_module(output)

set(LIAR_OUTPUT_HAVE_PIXELTOASTER_H FALSE)
set(_programfiles86 "ProgramFiles(x86)")
if (WIN32)
	find_path(D3D9_INCLUDE_DIR
		d3d9.h
		HINTS ENV DXSDK_DIR ENV Include
		PATHS "$ENV{${_programfiles86}}/Windows Kits/10/Include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/shared"
		PATHS "$ENV{${_programfiles86}}/Windows Kits/8.1/Include/shared"
		PATH_SUFFIXES Include
		DOC "path to d3d9.h"
		)
	if (D3D9_INCLUDE_DIR)
		target_include_directories(output
			BEFORE PRIVATE "${D3D9_INCLUDE_DIR}"
		)
		set(LIAR_OUTPUT_HAVE_PIXELTOASTER_H TRUE)
	endif()
else()
	if(APPLE)
		find_library(CORE_SERVICES_LIBRARY CoreServices)
		find_library(COCOA_LIBRARY Cocoa)
		find_package(OpenGL)
		mark_as_advanced(CORE_SERVICES_LIBRARY COCOA_LIBRARY)
		if(CORE_SERVICES_LIBRARY AND COCOA_LIBRARY AND OPENGL_FOUND)
			set(_apple_use_x11 OFF)
		else()
			set(_apple_use_x11 ON)
		endif()
		option(
			pixeltoaster_APPLE_USE_X11
			"PixelToaster uses X11 implementation on Apple"
			${_apple_use_x11}
			)
		set(PIXELTOASTER_APPLE_USE_X11 ${pixeltoaster_APPLE_USE_X11})
		target_link_libraries(output
			PRIVATE
				"${CORE_SERVICES_LIBRARY}"
				"${COCOA_LIBRARY}"
				${OPENGL_LIBRARIES}
		)
		if(NOT pixeltoaster_APPLE_USE_X11)
			target_sources(output
				PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PixelToaster/PixelToasterApple.mm"
			)
			set(LIAR_OUTPUT_HAVE_PIXELTOASTER_H TRUE)
		endif()
	endif()
	if (NOT LIAR_OUTPUT_HAVE_PIXELTOASTER_H)
		find_package(X11)
		if (X11_FOUND)
			target_include_directories(output
				BEFORE PRIVATE "${X11_INCLUDE_DIR}"
			)
			target_link_libraries(output
				PRIVATE ${X11_LIBRARIES})
			set(LIAR_OUTPUT_HAVE_PIXELTOASTER_H TRUE)
		endif()
	endif()
endif()
if (LIAR_OUTPUT_HAVE_PIXELTOASTER_H)
	target_include_directories(output
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PixelToaster"
	)
endif()

configure_file(
	output_config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/output_config.h
	)
target_include_directories(output
	PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
)

# display_impl.cpp does not use the precompiled header, we must disable it.
set_property(
	SOURCE display_impl.cpp
	PROPERTY
		SKIP_PRECOMPILE_HEADERS TRUE
)
