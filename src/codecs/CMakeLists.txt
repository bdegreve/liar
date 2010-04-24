macro(add_liar_codec name)
	file(GLOB ${name}_SRCS ${name}.cpp)
	set(${name}_LIBS libkernel ${ARGN})
	add_library(${name} MODULE 
		${${name}_SRCS}
		)
	set_target_properties(
		${name}
		PROPERTIES 
		COMPILE_FLAGS "${liar_CFLAGS}"
		)
	if (WIN32)
		set_target_properties(
			${name}
			PROPERTIES 
			SUFFIX ".pyd"
			)
	endif()
	target_link_libraries(${name} 
		${${name}_LIBS}
		)
	install(
		TARGETS ${name}
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
	if (WIN32)
	endif()
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

file(GLOB pyfiles "*.py")
install(
	FILES ${pyfiles}
	DESTINATION codecs
	)