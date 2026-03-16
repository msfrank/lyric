#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/lyric_metadata.h>
#include <tempo_test/tempo_test.h>

#include "base_build_fixture.h"
#include "lyric_build/build_attrs.h"
#include "lyric_build/metadata_writer.h"

class LyricMetadataTests : public BaseBuildFixture {};

TEST_F (LyricMetadataTests, DumpAndLoadJson)
{
    std::string expectedContentType("application/octet-stream");

    lyric_build::MetadataWriter writer;
    ASSERT_THAT (writer.configure(),  tempo_test::IsOk());
    ASSERT_THAT (writer.putAttr(lyric_build::kLyricBuildContentType, expectedContentType), tempo_test::IsOk());
    lyric_build::LyricMetadata expected;
    TU_ASSIGN_OR_RAISE (expected, writer.toMetadata());

    auto json = expected.dumpJson();
    auto actual = lyric_build::LyricMetadata::loadJson(json);
    ASSERT_TRUE (actual.isValid());

    ASSERT_EQ (1, actual.numAttrs());
    ASSERT_TRUE (actual.hasAttr(lyric_build::kLyricBuildContentType));
    std::string actualContentType;
    ASSERT_THAT (actual.parseAttr(lyric_build::kLyricBuildContentType, actualContentType), tempo_test::IsOk());
    ASSERT_EQ (expectedContentType, actualContentType);
}
