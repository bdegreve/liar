macro(add_liar_codec name)
	add_liar_module_ex(
		${name}
		SRCS "${name}.cpp"
		LIBS ${ARGN}
		DESTINATION liar/codecs
		)
endmacro()

# jpeglib: http://www.ijg.org/
if (TARGET CONAN_PKG::libjpeg)
	add_liar_codec(jpeglib CONAN_PKG::libjpeg)
else()
	find_package(JPEG)
	if (JPEG_FOUND)
		add_liar_codec(jpeglib JPEG::JPEG)
	endif()
endif()

# openexr 1.6.1: http://www.openexr.com/
if (TARGET CONAN_PKG::openexr)
	add_liar_codec(openexr CONAN_PKG::openexr)
endif()

if (TARGET CONAN_PKG::lodepng)
	add_liar_codec(lodepng CONAN_PKG::lodepng)
endif()

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
	DESTINATION liar/codecs
	)