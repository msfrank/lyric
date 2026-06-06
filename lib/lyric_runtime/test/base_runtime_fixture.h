#ifndef LYRIC_RUNTIME_BASE_RUNTIME_FIXTURE_H
#define LYRIC_RUNTIME_BASE_RUNTIME_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>

#include "lyric_runtime/static_loader.h"

class BaseRuntimeFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_test::LyricTester> tester;
    std::shared_ptr<lyric_runtime::StaticLoader> staticLoader;

    void SetUp() override;
};

#endif // LYRIC_RUNTIME_BASE_RUNTIME_FIXTURE_H