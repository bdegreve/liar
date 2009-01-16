file(GLOB tracers_HDRS *.h *.inl)
file(GLOB tracers_SRCS *.cpp)
add_library(tracers MODULE ${tracers_HDRS} ${tracers_SRCS})
target_link_libraries(tracers kernel_lib)