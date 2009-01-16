file(GLOB cameras_HDRS *.h *.inl)
file(GLOB cameras_SRCS *.cpp)
add_library(cameras MODULE ${cameras_HDRS} ${cameras_SRCS})
target_link_libraries(cameras kernel_lib)