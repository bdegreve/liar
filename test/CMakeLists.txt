set(test_SRCS)
file(GLOB _paths *.cpp)
foreach(_path ${_paths})
	if(${_path} MATCHES ".*/test_[a-zA-Z0-9_]+\\.cpp")
		list(APPEND test_SRCS "${_path}")
	endif()
endforeach()
unset(_path)
unset(_paths)

find_package(GTest REQUIRED)

add_executable(test_driver ${test_SRCS})
target_link_libraries(test_driver
	PRIVATE
		libkernel
		Python::Python
		GTest::gtest
		GTest::gtest_main
)
target_include_directories(test_driver
	PRIVATE
		"${liar_SOURCE_DIR}/src"
)
set_target_properties(test_driver
	PROPERTIES
		VS_DEBUGGER_ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Lass::lass>;%PATH%"
)

include(GoogleTest)
gtest_add_tests(
	TARGET test_driver
	TEST_LIST tests
)
set_tests_properties(
	${tests}
	PROPERTIES
		ENVIRONMENT "TEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
	)
if (WIN32)
	set_tests_properties(
		${tests}
		PROPERTIES
			ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Lass::lass>"
		)
endif()
