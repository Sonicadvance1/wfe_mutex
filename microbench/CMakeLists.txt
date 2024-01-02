add_executable(microbench_uncontended microbench_uncontended.cpp)
target_link_libraries(microbench_uncontended PRIVATE wfe_mutex)
# target_compile_options(microbench_uncontended PRIVATE "-mcpu=native")
set_property(TARGET microbench_uncontended PROPERTY C_STANDARD 17)
set_property(TARGET microbench_uncontended PROPERTY CXX_STANDARD 17)

