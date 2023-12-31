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
	list(APPEND SRCS
		src/implementations_arm64.c)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^armv7\.*")
	add_compile_options(-march=armv8-a+simd)
	set(_M_ARM_32 1)
	list(APPEND DEFINES _M_ARM_32=1)
	list(APPEND SRCS
		src/implementations_arm64.c)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
	set(_M_X86_64 1)
	list(APPEND DEFINES _M_X86_64=1)
	list(APPEND SRCS
		src/implementations_waitpkg.cpp
		src/implementations_mwaitx.cpp)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
	set(_M_X86_32 1)
	list(APPEND DEFINES _M_X86_32=1)
	list(APPEND SRCS
		src/implementations_waitpkg.cpp
		src/implementations_mwaitx.cpp)
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
	include(CTest)
	enable_testing()

	add_subdirectory(External/Catch2/)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/Catch2/CMake/")
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/External/Catch2/extras/")
	include(Catch)

	add_subdirectory(unittests/)
	add_subdirectory(microbench/)
endif()
