file(GLOB libkernel_SRCS 
	*.h 
	*.inl
	*.cpp
	)	
list_remove_regex(libkernel_SRCS "kernel_init.cpp$")
add_library(libkernel SHARED 
	${libkernel_SRCS}
	)
target_link_libraries(libkernel 
	lass 
	lass_python
	)
install(
	TARGETS libkernel 
	DESTINATION .
	)

add_liar_module_ex(kernel kernel_init.cpp)


