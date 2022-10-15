cmake_minimum_required(VERSION 3.14)

project(batd VERSION 0.1 LANGUAGES C DESCRIPTION "Battery threshold daemon for asahiLinux")

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not allowed!")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

add_subdirectory(src)
