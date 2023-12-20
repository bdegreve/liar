install(
	FILES "observer.tsv" "recovery_meng_simon.json"
	DESTINATION liar/data
	)

file(GLOB spdfiles "spd/*.spd")
install(
	FILES ${spdfiles}
	DESTINATION liar/data/spd
	)
