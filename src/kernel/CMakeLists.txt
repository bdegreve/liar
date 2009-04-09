file(GLOB kernelshared_HDRS 
	*.h 
	*.inl
	)
file(GLOB kernelshared_SRCS 
	*.cpp
	)	
list_remove_regex(kernelshared_SRCS "kernel_init.cpp$")
add_library(kernelshared SHARED 
	${kernelshared_HDRS} 
	${kernelshared_SRCS}
	)
target_link_libraries(kernelshared 
	lass
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
	kernelshared
	)



install(
	TARGETS kernel kernelshared 
	DESTINATION .
	)