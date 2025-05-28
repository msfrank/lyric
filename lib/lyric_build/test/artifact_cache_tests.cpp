#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/artifact_cache.h>
#include <lyric_build/memory_cache.h>

class ArtifactCache : public ::testing::Test {
protected:
    std::unique_ptr<lyric_build::ArtifactCache> cache;
    void SetUp() override {
        auto memoryCache = std::make_shared<lyric_build::MemoryCache>();
        std::make_unique<lyric_build::ArtifactCache>(
            std::static_pointer_cast<lyric_build::AbstractCache>(memoryCache));
    }
    void TearDown() override {
        cache.reset();
    }
};

TEST_F (ArtifactCache, CreateEntry) {

    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto createEntryStatus = cache->createFileEntry(id);
}
