add_executable(${PROJECT_NAME}
	batd.c
	conf.c
	debug.c
	main.c
)

target_include_directories(${PROJECT_NAME} PRIVATE include)
