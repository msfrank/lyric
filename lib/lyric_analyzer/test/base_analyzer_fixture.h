#ifndef LYRIC_ANALYZER_BASE_ANALYZER_FIXTURE_H
#define LYRIC_ANALYZER_BASE_ANALYZER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>

class BaseAnalyzerFixture : public ::testing::Test {
protected:
    std::unique_ptr<lyric_test::LyricTester> m_tester;

    void SetUp() override;
};

#endif // LYRIC_ANALYZER_BASE_ANALYZER_FIXTURE_H
