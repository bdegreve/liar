file(GLOB pyfiles "*.py")

add_custom_command(
	COMMENT "Creating venv"
	OUTPUT venv/pyvenv.cfg
	COMMAND Python::Interpreter
	ARGS -m venv venv
)

install(
	FILES ${pyfiles}
	DESTINATION liar/tools
	)
