find_package(LATEX QUIET)
if (LATEX_COMPILER)
	file(GLOB_RECURSE THE_GUIDE_SRCS 
		"latex/*.tex"
		"latex/*.bib"
		)
	set(_args "--quiet" "--include-directory=${CMAKE_CURRENT_SOURCE_DIR}/latex" "liar_the_guide")
	set(_commands)
	if (BIBTEX_COMPILER)
		list(APPEND _commands
			COMMAND "${LATEX_COMPILER}" ${_args}
			COMMAND "${BIBTEX_COMPILER}" ${_args}
			)
	endif()
	list(APPEND _commands
		COMMAND "${LATEX_COMPILER}" ${_args}
		COMMAND "${LATEX_COMPILER}" ${_args}
		)
	set(_work_dir )
	add_custom_target(THE_GUIDE
		${_commands}
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		SOURCES ${THE_GUIDE_SRCS}
		)
endif()

find_package(Doxygen QUIET)
if (DOXYGEN_EXECUTABLE)
	file(GLOB_RECURSE DOXYGEN_SRCS 
		"*.doxygen"
		)
	add_custom_target(DOXYGEN
		COMMAND "${DOXYGEN_EXECUTABLE}" ${DOXYGEN_SRCS}
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		SOURCES ${DOXYGEN_SRCS}
		)
endif()