file(GLOB pyfiles "*.py")

add_custom_command(
	COMMENT "Creating venv"
	OUTPUT venv/pyvenv.cfg
	COMMAND Python::Interpreter
	ARGS -m venv venv
)

if(WIN32)
	set(_python "venv/Scripts/python.exe")
else()
	set(_python "venv/bin/python")
endif()

# install requirements
add_custom_command(
	COMMENT "Installing cython"
	OUTPUT venv/.cython.stamp
	COMMAND "${_python}" -m pip install cython==3.0.2
	COMMAND "${CMAKE_COMMAND}" -E touch venv/.cython.stamp
	DEPENDS venv/pyvenv.cfg
)

macro(cythonize_liar_module pyfile)
	get_filename_component(_name "${pyfile}" NAME_WE)
	set(_cppfile "${CMAKE_CURRENT_BINARY_DIR}/${_name}.cpp")
	add_liar_module_ex(
		"${_name}"
		SRCS "${_cppfile}"
		DESTINATION liar/tools
		)
	add_custom_command(
		OUTPUT "${_cppfile}"
		COMMAND "${_python}"
		ARGS "-m" "cython" "-3" "--cplus" "--output-file" "${_cppfile}" "${pyfile}"
		MAIN_DEPENDENCY "${pyfile}"
		DEPENDS venv/.cython.stamp
	)
	list(REMOVE_ITEM pyfiles "${pyfile}")

	# put binary tools in a subdirectory of the normal output directory
	foreach(_prop LIBRARY_OUTPUT_DIRECTORY ARCHIVE_OUTPUT_DIRECTORY)
		get_target_property(_outputdir "${_name}" "${_prop}")
		if(_outputdir)
			if (CMAKE_CONFIGURATION_TYPES)
				# for MSVC and the like ...
				string(GENEX_STRIP "${_outputdir}" _without_genex)
				if(_outputdir STREQUAL _without_genex)
					# The string doesn't contain generator expressions.
					# cmake will automatically extend LIBRARY_OUTPUT_DIRECTORY
					# that's not what we want, but this would come after codecs
					# explictly adding $<CONFIG> will prevent this ...
					set(_outputdir "${_outputdir}/$<CONFIG>")
				endif()
			endif()
			set_target_properties("${_name}"
				PROPERTIES
					"${_prop}" "${_outputdir}/tools"
			)
		endif()
	endforeach()
	unset(_without_genex)
	unset(_outputdir)
	unset(_prop)
endmacro()

cythonize_liar_module("${CMAKE_CURRENT_SOURCE_DIR}/ply.py")

install(
	FILES ${pyfiles}
	DESTINATION liar/tools
	)
