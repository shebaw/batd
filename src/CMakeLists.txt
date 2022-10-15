add_executable(batd 
target_sources(
	${PROJECT_NAME}
	PRIVATE
	bat.c
	conf.c
	daemonize.c
	debug.c
	main.c
)

