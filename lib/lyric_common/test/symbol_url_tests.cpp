
#include <gtest/gtest.h>

#include <lyric_common/symbol_url.h>

TEST(SymbolUrl, ConstructRelativeSymbolUrlFromString)
{
    auto symbolUrl = lyric_common::SymbolUrl::fromString("#Bool");
    ASSERT_TRUE (symbolUrl.isValid());
    ASSERT_TRUE (symbolUrl.isRelative());
}

TEST(SymbolUrl, ConstructAbsoluteSymbolUrlFromString)
{
    auto symbolUrl = lyric_common::SymbolUrl::fromString("dev.zuri.bootstrap:/prelude#Bool");
    ASSERT_TRUE (symbolUrl.isValid());
    ASSERT_TRUE (symbolUrl.isAbsolute());
}
