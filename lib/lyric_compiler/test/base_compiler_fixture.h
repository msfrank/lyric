#ifndef LYRIC_COMPILER_BASE_COMPILER_FIXTURE_H
#define LYRIC_COMPILER_BASE_COMPILER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>

class BaseCompilerFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_test::LyricTester> m_tester;

    void SetUp() override;

    tempo_utils::Result<lyric_runtime::InterpreterExit> runComputationSet(
        absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
        const lyric_build::TaskKey &mainKey);
};

#endif // LYRIC_COMPILER_BASE_COMPILER_FIXTURE_H
