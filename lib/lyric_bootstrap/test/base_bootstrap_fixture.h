#ifndef LYRIC_BOOTSTRAP_BASE_BOOTSTRAP_FIXTURE_H
#define LYRIC_BOOTSTRAP_BASE_BOOTSTRAP_FIXTURE_H

#include <string>

#include <gtest/gtest.h>

#include <lyric_test/test_run.h>
#include <tempo_utils/result.h>

#include "lyric_test/lyric_tester.h"

class BaseBootstrapFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_test::LyricTester> tester;

    void SetUp() override;

    tempo_utils::Result<lyric_test::CompileModule> compileModule(const std::string &code);
    tempo_utils::Result<lyric_test::RunModule> runModule(const std::string &code);
};

#endif // LYRIC_BOOTSTRAP_BASE_BOOTSTRAP_FIXTURE_H