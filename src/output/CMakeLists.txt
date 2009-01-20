find_path(
	pixeltoaster_INCLUDE_DIR 
	PixelToaster.h
	)
if (pixeltoaster_INCLUDE_DIR)
	include_directories(
		${pixeltoaster_INCLUDE_DIR}
		)
	option(
		pixeltoaster_HAVE_DIRTYBOX 
		"PixelToaster supports update with dirty box" 
		OFF)
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
