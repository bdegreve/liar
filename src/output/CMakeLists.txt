set (_libraries)

find_path(
	pixeltoaster_INCLUDE_DIR 
	PixelToaster.h
	)
if (pixeltoaster_INCLUDE_DIR)
	set(_ok TRUE)
	if (UNIX)
		find_package(X11)
		if (X11_FOUND)
			include_directories(
				${X11_INCLUDE_DIR}
				)
			list(APPEND output_LIBS ${X11_LIBRARIES})
		else()
			set(_ok FALSE)
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
	${pixeltoaster_INCLUDE_DIR}
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

