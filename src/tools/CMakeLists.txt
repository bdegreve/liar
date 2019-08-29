file(GLOB pyfiles "*.py")
install(
	FILES ${pyfiles}
	DESTINATION liar/tools
	)