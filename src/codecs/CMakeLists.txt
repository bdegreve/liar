macro(add_liar_codec name)
	add_liar_module_ex(
		"${name}"
		SRCS "${name}.cpp"
		LIBS ${ARGN}
		DESTINATION liar/codecs
		)

	# put codecs in a subdirectory of the normal output directory
	foreach(_prop LIBRARY_OUTPUT_DIRECTORY ARCHIVE_OUTPUT_DIRECTORY)
		get_target_property(_outputdir "${name}" "${_prop}")
		if(_outputdir)
			if (CMAKE_CONFIGURATION_TYPES)
				# for MSVC and the like ...
				string(GENEX_STRIP "${_outputdir}" _without_genex)
				if(_outputdir STREQUAL _without_genex)
					# The string doesn't contain generator expressions.
					# cmake will automatically extend LIBRARY_OUTPUT_DIRECTORY
					# that's not what we want, but this would come after codecs
					# explictly adding $<CONFIG> will prevent this ...
					set(_outputdir "${_outputdir}/$<CONFIG>")
				endif()
			endif()
			set_target_properties("${name}"
				PROPERTIES
					"${_prop}" "${_outputdir}/codecs"
			)
		endif()
	endforeach()
	unset(_without_genex)
	unset(_outputdir)
	unset(_prop)
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

# openexr: http://www.openexr.com/
find_package(OpenEXR)
if (OpenEXR_FOUND)
	add_liar_codec(openexr OpenEXR::OpenEXR)
endif()

find_package(lodepng)
if(lodepng_FOUND)
	add_liar_codec(lodepng lodepng::lodepng)
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
