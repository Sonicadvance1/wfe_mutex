create_test_sourcelist(TESTS
	common_tests.c
	init_test.c
	cpp_test.cpp
	mutex_test.cpp
	cpp_mutex_test.cpp
	)

file(GLOB TEST_SOURCES *.c *.cpp)

foreach(TEST ${TEST_SOURCES})
	get_filename_component(TestName ${TEST} NAME_WE)
	add_executable(${TestName} ${TEST})
	target_link_libraries(${TestName} PRIVATE wfe_mutex Catch2::Catch2WithMain)
	set_property(TARGET ${TestName} PROPERTY C_STANDARD 17)
	set_property(TARGET ${TestName} PROPERTY CXX_STANDARD 17)

	catch_discover_tests(${TestName} TEST_SUFFIX ".${TestName}.test")
endforeach()

execute_process(COMMAND "nproc" OUTPUT_VARIABLE CORES)
string(STRIP ${CORES} CORES)

add_custom_target(
	tests
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	USES_TERMINAL
	COMMAND "ctest" "--output-on-failure" "-j${CORES}" "-R" "\.*.test")

foreach(TEST ${TEST_SOURCES})
	get_filename_component(TestName ${TEST} NAME_WE)
	add_dependencies(tests ${TestName})
endforeach()
