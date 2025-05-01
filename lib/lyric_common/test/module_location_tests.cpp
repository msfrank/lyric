
#include <gtest/gtest.h>

#include <lyric_common/symbol_url.h>

TEST(ModuleLocation, ConstructAbsoluteModuleLocation)
{
    auto moduleLocation = lyric_common::ModuleLocation::fromString("http://origin/foo/bar");
    ASSERT_TRUE (moduleLocation.isValid());
    ASSERT_TRUE (moduleLocation.isAbsolute());
    ASSERT_TRUE (moduleLocation.hasScheme());
    ASSERT_TRUE (moduleLocation.hasOrigin());
    ASSERT_TRUE (moduleLocation.hasPathParts());
    ASSERT_EQ ("http://origin/foo/bar", moduleLocation.toString());
}

TEST(ModuleLocation, ConstructRelativeModuleLocation)
{
    auto moduleLocation = lyric_common::ModuleLocation::fromString("/foo/bar");
    ASSERT_TRUE (moduleLocation.isValid());
    ASSERT_TRUE (moduleLocation.isRelative());
    ASSERT_FALSE (moduleLocation.hasScheme());
    ASSERT_FALSE (moduleLocation.hasOrigin());
    ASSERT_TRUE (moduleLocation.hasPathParts());
    ASSERT_EQ ("/foo/bar", moduleLocation.toString());
}
