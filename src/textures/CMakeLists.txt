file(GLOB textures_HDRS *.h *.inl)
file(GLOB textures_SRCS *.cpp)
add_library(textures MODULE ${textures_HDRS} ${textures_SRCS})
target_link_libraries(textures kernel_lib)