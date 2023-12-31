create_test_sourcelist(TESTS
	common_tests.c
	init_test.c
	cpp_test.cpp
	mutex_test.c
	cpp_mutex_test.cpp
	)

add_executable(tests ${TESTS})
target_link_libraries(tests wfe_mutex)
set_property(TARGET tests PROPERTY C_STANDARD 17)
set_property(TARGET tests PROPERTY CXX_STANDARD 17)

foreach (Test ${TESTS})
	get_filename_component(TestName ${Test} NAME_WE)
	add_test(NAME ${TestName} COMMAND tests ${TestName})
endforeach()
