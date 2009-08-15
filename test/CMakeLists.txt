file(GLOB fnames *.cpp)
foreach(fname ${fnames})
	if(${fname} MATCHES "/test_[a-zA-Z0-9_]+\\.cpp")
		get_filename_component(test "${fname}" NAME)
		list(APPEND tests "${test}")
	endif()
endforeach()

create_test_sourcelist(test_SRCS "main.cpp" ${tests})

include_directories(../src)
add_executable(test_driver ${test_SRCS})
target_link_libraries(test_driver kernelshared)

foreach(test ${tests})
	get_filename_component(name "${test}" NAME_WE)
	add_test(${name} test_driver ${name})
endforeach()


