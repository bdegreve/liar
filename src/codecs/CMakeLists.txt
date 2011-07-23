macro(add_liar_codec name)
	add_liar_module_ex(
		${name}
		SRCS "${name}.cpp"
		LIBS ${ARGN}
		DESTINATION codecs
		)
endmacro()

# jpeglib: http://www.ijg.org/
find_package(JPEG)
mark_as_advanced(CLEAR JPEG_INCLUDE_DIR JPEG_LIBRARY)
if (JPEG_FOUND)
	include_directories(
		${JPEG_INCLUDE_DIR}
		)
	add_liar_codec(jpeglib ${JPEG_LIBRARIES})
endif()

# openexr 1.6.1: http://www.openexr.com/
find_package(OpenEXR COMPONENTS Half Iex IlmImf IlmThread)
if (OPENEXR_FOUND)
	include_directories(
		${OpenEXR_INCLUDE_DIR}
		)
	add_definitions(
		${OpenEXR_DEFINITIONS}
		)
	add_liar_codec(openexr ${OpenEXR_LIBRARIES})
	install(
		FILES ${OpenEXR_REDIST}
		DESTINATION codecs
		CONFIGURATIONS RELEASE MINSIZEREL RELWITHDEBINFO
		)
	install(
		FILES ${OpenEXR_DEBUG_REDIST}
		DESTINATION codecs
		CONFIGURATIONS DEBUG 
		)
ENDIF()

# libtiff: http://www.remotesensing.org/libtiff/
find_package(TIFF)
mark_as_advanced(CLEAR TIFF_INCLUDE_DIR TIFF_LIBRARY)
if (TIFF_FOUND)
	include_directories(
		${TIFF_INCLUDE_DIR}
		)
	#add_liar_codec(tiff ${TIFF_LIBRARIES})
	if (WIN32)
	endif()
endif()

file(GLOB pyfiles "*.py")
install(
	FILES ${pyfiles}
	DESTINATION codecs
	)