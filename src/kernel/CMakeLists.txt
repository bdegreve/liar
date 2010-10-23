file(GLOB libkernel_HDRS 
	*.h 
	*.inl
	)
file(GLOB libkernel_SRCS 
	*.cpp
	)	
list_remove_regex(libkernel_SRCS "kernel_init.cpp$")
add_library(libkernel SHARED 
	${libkernel_HDRS} 
	${libkernel_SRCS}
	)
target_link_libraries(libkernel 
	lass 
	lass_python
	)



add_library(kernel MODULE 
	kernel_init.cpp
	)
if (WIN32)
	set_target_properties(
		kernel
		PROPERTIES 
		SUFFIX ".pyd"
		)
endif()
target_link_libraries(kernel 
	libkernel
	)

set_target_properties(
	libkernel kernel
	PROPERTIES 
	COMPILE_FLAGS "${liar_CFLAGS}"
	)

install(
	TARGETS kernel libkernel 
	DESTINATION .
	)