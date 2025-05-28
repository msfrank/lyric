#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/memory_cache.h>
#include <tempo_test/tempo_test.h>
#include <tempo_utils/memory_bytes.h>

#include "lyric_build/metadata_writer.h"

class MemoryCache : public ::testing::Test {
protected:
    std::shared_ptr<lyric_build::MemoryCache> cache;
    void SetUp() override {
        cache = std::make_shared<lyric_build::MemoryCache>();
    }
    void TearDown() override {
        cache.reset();
    }
};

TEST_F (MemoryCache, StoreNewContentAndMetadata)
{
    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());

    auto content = tempo_utils::MemoryBytes::copy("hello world");
    auto storeContentStatus = cache->storeContent(id, content);
    ASSERT_THAT (storeContentStatus, tempo_test::IsOk());

    lyric_build::MetadataWriter writer;
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());
    auto createMetadataResult = writer.toMetadata();
    ASSERT_THAT (createMetadataResult, tempo_test::IsResult());

    auto metadata = createMetadataResult.getResult();
    auto storeMetadataStatus = cache->storeMetadata(id, metadata);
    ASSERT_THAT (storeMetadataStatus, tempo_test::IsOk());
}