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

if(NOT DEFINED liar_PYTHON_BASE_PREFIX)
	if (NOT DEFINED _liar_PYTHON_BASE_PREFIX OR NOT Python_EXECUTABLE STREQUAL _liar_PYTHON_PREFIX_EXECUTABLE)
		message(STATUS "Looking for sys.base_prefix")
		execute_process(COMMAND "${Python_EXECUTABLE}" -c "import sys; sys.stdout.write(sys.base_prefix.replace('\\\\', '/'))"
			RESULT_VARIABLE __liar_PYTHON_PREFIX_RESULT
			OUTPUT_VARIABLE liar_PYTHON_BASE_PREFIX
			ERROR_VARIABLE __liar_PYTHON_PREFIX_ERROR
		)
		if(NOT __liar_PYTHON_PREFIX_RESULT)
			set(_liar_PYTHON_BASE_PREFIX "${liar_PYTHON_BASE_PREFIX}" CACHE INTERNAL "sys.base_prefix")
			set(_liar_PYTHON_PREFIX_EXECUTABLE "${Python_EXECUTABLE}" CACHE INTERNAL "sys.base_prefix")
			message(STATUS "Looking for sys.base_prefix - found: ${liar_PYTHON_BASE_PREFIX}")
		else()
			set(liar_PYTHON_BASE_PREFIX)
			message(STATUS "Looking for sys.base_prefix - failed\n${__liar_PYTHON_PREFIX_ERROR}")
		endif()
	else()
		set(liar_PYTHON_BASE_PREFIX "${_liar_PYTHON_BASE_PREFIX}")
	endif()
endif()


add_executable(test_driver main.cpp ${test_SRCS})
target_link_libraries(test_driver
	PRIVATE
		libkernel
		Python::Python
		GTest::gtest
)
target_include_directories(test_driver
	PRIVATE
		"${liar_SOURCE_DIR}/src"
)
target_compile_definitions(test_driver
	PRIVATE
		"ROOT_DIR=${liar_SOURCE_DIR}"
		"TEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
		"BIN_DIR=$<TARGET_FILE_DIR:libkernel>"
		"PYTHON_BASE_PREFIX=${liar_PYTHON_BASE_PREFIX}"
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

get_cmake_property(_is_multi_config GENERATOR_IS_MULTI_CONFIG)
if(_is_multi_config)
	set(_liar_pth "${liar_BINARY_DIR}/$<CONFIGURATION>/liar.pth")
else()
	set(_liar_pth "${liar_BINARY_DIR}/liar.pth")
endif()
file(
	GENERATE
	OUTPUT "${_liar_pth}"
	CONTENT "import os, importlib.util; \
_liar_spec = importlib.util.spec_from_file_location('path_hooks', os.path.join('${CMAKE_CURRENT_SOURCE_DIR}', 'path_hooks.py')); \
assert _liar_spec; \
_liar_path_hooks = importlib.util.module_from_spec(_liar_spec); \
_liar_spec.loader.exec_module(_liar_path_hooks); \
_liar_path_hooks.setup_liar_hooks(root_dir='${liar_SOURCE_DIR}', bin_dir='$<TARGET_FILE_DIR:libkernel>'); \
del _liar_spec, _liar_path_hooks"
)
unset(_liar_pth)
unset(_is_multi_config)
