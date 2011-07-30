file(GLOB libkernel_SRCS 
	*.h 
	*.inl
	*.cpp
	)	
list_remove_regex(libkernel_SRCS "kernel_init.cpp$")
add_library(libkernel SHARED 
	${libkernel_SRCS}
	)
if(APPLE)
	set_target_properties(
		libkernel
		PROPERTIES 
		INSTALL_NAME_DIR "@rpath"
		)
endif()
target_link_libraries(libkernel 
	lass_python
	)
install(
	TARGETS libkernel 
	DESTINATION .
	)

add_liar_module_ex(kernel kernel_init.cpp)


