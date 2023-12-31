cmake_minimum_required(VERSION 3.14)
project(wfe_mutex)
OPTION(ENABLE_INSTALL "Enables installing of the library" ON)
OPTION(BUILD_TESTS "Enables building tests" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (SRCS
	src/detect.c
	src/implementations.c
	src/wfe_mutex.c)

set (DEFINES )
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64|^arm64|^armv8\.*")
	set(_M_ARM_64 1)
	list(APPEND DEFINES _M_ARM_64=1)
	list(APPEND SRCS implementations_arm64.c)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
	set(_M_X86_64 1)
	list(APPEND DEFINES _M_X86_64=1)
	list(APPEND SRCS
		src/implementations_waitpkg.c
		src/implementations_mwaitx.c)
endif()

add_compile_options(-Wall)

add_library(wfe_mutex STATIC ${SRCS})
add_compile_definitions(${DEFINES})
target_include_directories(wfe_mutex PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_property(TARGET wfe_mutex PROPERTY C_STANDARD 17)

if (ENABLE_INSTALL)
	install (TARGETS wfe_mutex
		ARCHIVE
			DESTINATION lib
			COMPONENT Libraries)

	install (DIRECTORY include/wfe_mutex
		DESTINATION include
		COMPONENT Development)
endif()

if (BUILD_TESTS)
	add_subdirectory(unittests/)
endif()
