macro(add_liar_codec name libraries)
	file(GLOB ${name}_SRCS ${name}.cpp)
	set(${name}_LIBS kernelshared ${libraries})
	add_library(${name} MODULE 
		${${name}_SRCS}
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

include(FindJPEG)
mark_as_advanced(CLEAR JPEG_INCLUDE_DIR JPEG_LIBRARY)
if (JPEG_FOUND)
	include_directories(
		${JPEG_INCLUDE_DIR}
		)
	add_liar_codec(jpeg ${JPEG_LIBRARIES})
	if (WIN32)
	endif()
endif()

file(GLOB pyfiles "*.py")
install(
	FILES ${pyfiles}
	DESTINATION codecs
	)