#if (MSVC)
#	set (_cflags "/Ox")
#	foreach(_config RELEASE RELWITHDEBINFO)
#		set(CMAKE_C_FLAGS_${_config} "${CMAKE_C_FLAGS_${_config}} ${_cflags}")
#		set(CMAKE_CXX_FLAGS_${_config} "${CMAKE_CXX_FLAGS_${_config}} ${_cflags}")
#	endforeach()
#endif()

add_subdirectory(kernel)

add_subdirectory(cameras)
add_subdirectory(codecs)
add_subdirectory(output)
add_subdirectory(samplers)
add_subdirectory(scenery)
add_subdirectory(shaders)
add_subdirectory(textures)
add_subdirectory(tools)
add_subdirectory(tracers)

file(GLOB pyfiles "*.py")
install(
	FILES ${pyfiles}
	DESTINATION .
	)
