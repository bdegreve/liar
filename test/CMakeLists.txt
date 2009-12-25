file(GLOB paths *.cpp)
foreach(path ${paths})
	if(${path} MATCHES ".*/test_[a-zA-Z0-9_]+\\.cpp")
		get_filename_component(fname "${path}" NAME)
		list(APPEND test_files "${fname}")
	endif()
endforeach()

create_test_sourcelist(test_SRCS "main.cpp" ${test_files})

include_directories(../src)
add_executable(test_driver ${test_SRCS})
target_link_libraries(test_driver kernelshared)

foreach(fname ${test_files})
	get_filename_component(test "${fname}" NAME_WE)
	add_test(
		NAME ${test} 
		COMMAND 
			"${Lass_PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/testenv.py"
			"--with-library=$<TARGET_FILE:lass>"
			"$<TARGET_FILE:test_driver>" "${test}"
		)
	list(APPEND tests ${test})
endforeach()

set_tests_properties(
	${tests}
	PROPERTIES
	ENVIRONMENT "TEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
	)