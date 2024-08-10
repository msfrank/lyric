#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <string>

#include <lyric_test/test_run.h>
#include <tempo_utils/result.h>

tempo_utils::Result<lyric_test::CompileModule> compileModule(const std::string &code);

tempo_utils::Result<lyric_test::RunModule> runModule(const std::string &code);

#endif // TEST_HELPERS_H
