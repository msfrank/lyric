#cmake_minimum_required(VERSION 2.8.11)
#
#enable_testing()
#
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../")
#
## define the testing libraries to link against
#set(TEST_LIBRARIES CONAN_PKG::GtestDevel Qt5::Test)
#if (APPLE)
#    set(TEST_LIBRARIES ${TEST_SANITIZER_LIBS} ${TEST_LIBRARIES} -lpthread)
#elseif(WIN32)
#else()
#    set(TEST_LIBRARIES ${TEST_SANITIZER_LIBS} ${TEST_LIBRARIES} -lpthread)
#endif()
#
## define unit tests
#
##add_executable(test_builtin_element test_builtin_element.cpp)
##target_link_libraries(test_builtin_element ${TEST_LIBRARIES} FathomNotebookJit)
##add_test(test_builtin_element test_builtin_element)
#
#add_executable(test_builtin_assignment test_builtin_assignment.cpp)
#target_link_libraries(test_builtin_assignment ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_assignment_tests COMMAND test_builtin_assignment)
#
#add_executable(test_builtin_boolean test_builtin_boolean.cpp)
#target_link_libraries(test_builtin_boolean ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_boolean_tests COMMAND test_builtin_boolean)
#
#add_executable(test_builtin_build test_builtin_build.cpp)
#target_link_libraries(test_builtin_build ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_build_tests COMMAND test_builtin_build)
#
#add_executable(test_builtin_conditional test_builtin_conditional.cpp)
#target_link_libraries(test_builtin_conditional ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_conditional_tests COMMAND test_builtin_conditional)
#
#add_executable(test_builtin_definition test_builtin_definition.cpp)
#target_link_libraries(test_builtin_definition ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_definition_tests COMMAND test_builtin_definition)
#
#add_executable(test_builtin_float test_builtin_float.cpp)
#target_link_libraries(test_builtin_float ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_float_tests COMMAND test_builtin_float)
#
#add_executable(test_builtin_integer test_builtin_integer.cpp)
#target_link_libraries(test_builtin_integer ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_integer_tests COMMAND test_builtin_integer)
#
#add_executable(test_builtin_loop test_builtin_loop.cpp)
#target_link_libraries(test_builtin_loop ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_loop_tests COMMAND test_builtin_loop)
#
#add_executable(test_builtin_lambda test_builtin_lambda.cpp)
#target_link_libraries(test_builtin_lambda ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_lambda_tests COMMAND test_builtin_lambda)
#
#add_executable(test_builtin_object test_builtin_object.cpp)
#target_link_libraries(test_builtin_object ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_object_tests COMMAND test_builtin_object)
#
#add_executable(test_builtin_map test_builtin_map.cpp)
#target_link_libraries(test_builtin_map ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_map_tests COMMAND test_builtin_map)
#
#add_executable(test_builtin_seq test_builtin_seq.cpp)
#target_link_libraries(test_builtin_seq ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_seq_tests COMMAND test_builtin_seq)
#
#add_executable(test_builtin_string test_builtin_string.cpp)
#target_link_libraries(test_builtin_string ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME builtin_string_tests COMMAND test_builtin_string)
#
##add_executable(test_bytecode_interpreter test_bytecode_interpreter.cpp)
##target_link_libraries(test_bytecode_interpreter ${TEST_LIBRARIES} FathomNotebookJit)
##add_test(NAME bytecode_interpreter_tests COMMAND test_bytecode_interpreter)
#
#add_executable(test_fir_compiler test_fir_compiler.cpp)
#target_link_libraries(test_fir_compiler ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME fir_compiler_tests COMMAND test_fir_compiler)
#
#add_executable(test_type_universe test_type_universe.cpp)
#target_link_libraries(test_type_universe ${TEST_LIBRARIES} FathomNotebookJit)
#add_test(NAME type_universe_tests COMMAND test_type_universe)
