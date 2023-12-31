create_test_sourcelist(TESTS
	common_tests.c
	init_test.c
	cpp_test.cpp
	)

add_executable(tests ${TESTS})
target_link_libraries(tests wfe_mutex)

foreach (Test ${TESTS})
	get_filename_component(TestName ${Test} NAME_WE)
	add_test(NAME, ${TestName} COMMAND tests ${TestName})
endforeach()
