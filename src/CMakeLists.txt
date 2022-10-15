add_executable(${PROJECT_NAME}
	bat.c
	#conf.c
	daemonize.c
	debug.c
	main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
