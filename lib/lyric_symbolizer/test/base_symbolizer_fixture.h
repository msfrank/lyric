#ifndef LYRIC_SYMBOLIZER_BASE_SYMBOLIZER_FIXTURE_H
#define LYRIC_SYMBOLIZER_BASE_SYMBOLIZER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>

class BaseSymbolizerFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_test::LyricTester> m_tester;

    void SetUp() override;
};

#endif // LYRIC_SYMBOLIZER_BASE_SYMBOLIZER_FIXTURE_H
