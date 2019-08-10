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
if (JPEG_FOUND)
	include_directories(
		${JPEG_INCLUDE_DIR}
		)
	add_liar_codec(jpeglib ${JPEG_LIBRARIES})
endif()

# openexr 1.6.1: http://www.openexr.com/
find_package(OpenEXR)
if (OPENEXR_FOUND)
	add_liar_codec(openexr OpenEXR::IlmImf)
	set_property(
		TARGET openexr
		APPEND PROPERTY INCLUDE_DIRECTORIES
		${OpenEXR_INCLUDE_DIRS}
	)
	set_property(
		TARGET openexr
		APPEND_STRING PROPERTY COMPILE_OPTIONS
		${OpenEXR_DEFINITIONS}
	)
	if (OpenEXR_BUILD)
		add_dependencies(openexr external_openexr)
	endif()
	install(
		FILES ${OpenEXR_REDISTS}
		DESTINATION codecs
		CONFIGURATIONS RELEASE MINSIZEREL RELWITHDEBINFO
		)
	install(
		FILES ${OpenEXR_DEBUG_REDISTS}
		DESTINATION codecs
		CONFIGURATIONS DEBUG 
		)
ENDIF()

# libtiff: http://www.remotesensing.org/libtiff/
find_package(TIFF)
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