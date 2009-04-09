add_subdirectory(kernel)

add_subdirectory(cameras)
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
