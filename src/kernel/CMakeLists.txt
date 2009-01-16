file(GLOB kernel_lib_HDRS *.h *.inl)
file(GLOB kernel_lib_SRCS *.cpp)

list_remove_regex(kernel_lib_SRCS "kernel_init.cpp$")

add_library(kernel_lib SHARED ${kernel_lib_HDRS} ${kernel_lib_SRCS})
target_link_libraries(kernel_lib lass)

add_library(kernel MODULE kernel_init.cpp)
target_link_libraries(kernel kernel_lib)