set (_libraries)
set (_srcs)

find_path(
	pixeltoaster_INCLUDE_DIR 
	PixelToaster.h
	DOC "path to PixelToaster.h"
	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/PixelToaster"
	)
mark_as_advanced(pixeltoaster_INCLUDE_DIR) 
if (pixeltoaster_INCLUDE_DIR)
	set(_ok FALSE)
	if (WIN32)
		find_path(D3D9_INCLUDE_DIR
			d3d9.h
			HINTS ENV DXSDK_DIR ENV Include
			PATH_SUFFIXES Include
			DOC "path to d3d9.h"
			)
		if (D3D9_INCLUDE_DIR)
			include_directories(
				BEFORE ${D3D9_INCLUDE_DIR}
				)
			set(_ok TRUE)
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
				list(APPEND output_LIBS 
					${CORE_SERVICES_LIBRARY}
					${COCOA_LIBRARY}
					${OPENGL_LIBRARIES}
					)		
			if(NOT pixeltoaster_APPLE_USE_X11)
				list(APPEND output_SRCS 
					"PixelToaster/PixelToasterApple.mm"
					)
				set(_ok TRUE)
			endif()
		endif()
		if (NOT _ok)
			find_package(X11)
			if (X11_FOUND)
				include_directories(
					BEFORE ${X11_INCLUDE_DIR}
					)
				list(APPEND output_LIBS 
					${X11_LIBRARIES}
					)
				set(_ok TRUE)
			endif()
		endif()
	endif()
	if (_ok)
		include_directories(
			${pixeltoaster_INCLUDE_DIR}
			)
		if ("pixeltoaster_HAVE_RECTANGLE" MATCHES "^pixeltoaster_HAVE_RECTANGLE$")
			message(STATUS "Looking for PixelToaster::Rectangle")
			configure_file(
				"${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_pixeltoaster_rectangle.cpp.in"
				"${CMAKE_CURRENT_BINARY_DIR}/cmake/check_pixeltoaster_rectangle.cpp"
				NOW)
			try_compile(
				pixeltoaster_HAVE_RECTANGLE
				"${CMAKE_CURRENT_BINARY_DIR}"
				"${CMAKE_CURRENT_BINARY_DIR}/cmake/check_pixeltoaster_rectangle.cpp"
				)
			if(pixeltoaster_HAVE_RECTANGLE)
				message(STATUS "Looking for PixelToaster::Rectangle - found")
			else()
				message(STATUS "Looking for PixelToaster::Rectangle - not found")
			endif()
		endif()
		option(
			pixeltoaster_HAVE_DIRTYBOX 
			"PixelToaster supports update with dirty box" 
			${pixeltoaster_HAVE_RECTANGLE})
	endif()
endif()

set(LIAR_OUTPUT_HAVE_PIXELTOASTER_H 
	${_ok}
	)
set(LIAR_OUTPUT_HAVE_PIXELTOASTER_DIRTYBOX 
	${pixeltoaster_HAVE_DIRTYBOX}
	)
configure_file(
	output_config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/output_config.h
	)
include_directories(
	BEFORE ${CMAKE_CURRENT_BINARY_DIR}
	)

add_liar_module(output)

