file(GLOB samplers_HDRS *.h *.inl)
file(GLOB samplers_SRCS *.cpp)
add_library(samplers MODULE ${samplers_HDRS} ${samplers_SRCS})
target_link_libraries(samplers kernel_lib)