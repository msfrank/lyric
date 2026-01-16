#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/abstract_cache.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/memory_cache.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/rocksdb_cache.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/tempdir_maker.h>

class CacheFixture {
public:
    virtual ~CacheFixture() = default;
    virtual void initialize() {};
    virtual void cleanup() {};
    virtual std::shared_ptr<lyric_build::AbstractCache> getCache() const = 0;
};

class RocksdbCache : public CacheFixture {
public:
    void initialize() override {
        tempo_utils::TempdirMaker dbPathMaker("rocksdb_cache.XXXXXXXX");
        TU_RAISE_IF_NOT_OK (dbPathMaker.getStatus());
        dbPath = dbPathMaker.getTempdir();
        cache = std::make_shared<lyric_build::RocksdbCache>(dbPath, false);
        TU_RAISE_IF_NOT_OK (cache->initializeCache());
    }
    void cleanup() override {
        cache.reset();
        std::filesystem::remove_all(dbPath);
    }
    std::shared_ptr<lyric_build::AbstractCache> getCache() const override {
        return cache;
    }
private:
    std::filesystem::path dbPath;
    std::shared_ptr<lyric_build::RocksdbCache> cache;
};

class MemoryCache : public CacheFixture {
public:
    void initialize() override {
        cache = std::make_shared<lyric_build::MemoryCache>();
    }
    std::shared_ptr<lyric_build::AbstractCache> getCache() const override {
        return cache;
    }
private:
    std::shared_ptr<lyric_build::MemoryCache> cache;
};

template <class T>
CacheFixture* CreateCacheFixture();

template <>
CacheFixture* CreateCacheFixture<RocksdbCache>()
{
    return new RocksdbCache();
}

template <>
CacheFixture* CreateCacheFixture<MemoryCache>()
{
    return new MemoryCache();
}

template <class T>
class AbstractCache : public testing::Test {
protected:
    CacheFixture* const fixture;

    AbstractCache() : fixture(CreateCacheFixture<T>()) {}
    ~AbstractCache() override { delete fixture; }

    void SetUp() override { fixture->initialize(); }
    void TearDown() override { fixture->cleanup(); }
};

typedef ::testing::Types<MemoryCache, RocksdbCache> Implementations;
TYPED_TEST_SUITE(AbstractCache, Implementations);

TYPED_TEST (AbstractCache, DeclareArtifact)
{
    auto cache = this->fixture->getCache();
    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());
    ASSERT_TRUE (cache->hasArtifact(id));
}

TYPED_TEST (AbstractCache, StoreAndLoadMetadata)
{
    auto cache = this->fixture->getCache();
    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());
    ASSERT_TRUE (cache->hasArtifact(id));

    lyric_build::MetadataWriter writer;
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());
    writer.putAttr(lyric_build::kLyricBuildGeneration, generation.toString());
    auto createMetadataResult = writer.toMetadata();
    ASSERT_THAT (createMetadataResult, tempo_test::IsResult());

    auto metadata = createMetadataResult.getResult();
    auto storeMetadataStatus = cache->storeMetadata(id, metadata);
    ASSERT_THAT (storeMetadataStatus, tempo_test::IsOk());

    auto loadMetadataResult = cache->loadMetadata(id);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    metadata = loadMetadataResult.getResult();

    std::string value;
    ASSERT_THAT (metadata.parseAttr(lyric_build::kLyricBuildGeneration, value), tempo_test::IsOk());
    ASSERT_EQ (generation.toString(), value);
}

TYPED_TEST (AbstractCache, StoreAndLoadContent)
{
    auto cache = this->fixture->getCache();
    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());

    auto content = tempo_utils::MemoryBytes::copy("hello world");
    auto storeContentStatus = cache->storeContent(id, content);
    ASSERT_THAT (storeContentStatus, tempo_test::IsOk());

    auto loadContentResult = cache->loadContent(id);
    ASSERT_THAT (loadContentResult, tempo_test::IsResult());
    auto value = loadContentResult.getResult();

    std::string_view contentString((const char *) content->getData(), content->getSize());
    std::string_view valueString((const char *) value->getData(), value->getSize());
    ASSERT_EQ (contentString, valueString);
}

TYPED_TEST (AbstractCache, LinkArtifactAndLoadMetadata)
{
    auto cache = this->fixture->getCache();
    auto generation = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());

    lyric_build::MetadataWriter writer;
    ASSERT_THAT (writer.configure(), tempo_test::IsOk());
    writer.putAttr(lyric_build::kLyricBuildGeneration, generation.toString());
    auto createMetadataResult = writer.toMetadata();
    ASSERT_THAT (createMetadataResult, tempo_test::IsResult());

    auto metadata = createMetadataResult.getResult();
    auto storeMetadataStatus = cache->storeMetadata(id, metadata);
    ASSERT_THAT (storeMetadataStatus, tempo_test::IsOk());

    lyric_build::ArtifactId linkId(generation, hash, tempo_utils::UrlPath::fromString("/link"));
    ASSERT_THAT (cache->linkArtifact(linkId, id), tempo_test::IsOk());

    auto loadMetadataResult = cache->loadMetadataFollowingLinks(linkId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    metadata = loadMetadataResult.getResult();

    std::string value;
    ASSERT_THAT (metadata.parseAttr(lyric_build::kLyricBuildGeneration, value), tempo_test::IsOk());
    ASSERT_EQ (generation.toString(), value);
}

TYPED_TEST (AbstractCache, LinkArtifactOverridingMetadataAndLoadMetadata)
{
    auto cache = this->fixture->getCache();

    auto generation1 = tempo_utils::UUID::randomUUID();
    auto hash = "foo";
    lyric_build::ArtifactId id(generation1, hash, tempo_utils::UrlPath::fromString("/file"));

    auto declareArtifactStatus = cache->declareArtifact(id);
    ASSERT_THAT (declareArtifactStatus, tempo_test::IsOk());

    lyric_build::MetadataWriter writer1;
    ASSERT_THAT (writer1.configure(), tempo_test::IsOk());
    writer1.putAttr(lyric_build::kLyricBuildGeneration, generation1.toString());
    auto createMetadataResult = writer1.toMetadata();
    ASSERT_THAT (createMetadataResult, tempo_test::IsResult());

    auto metadata = createMetadataResult.getResult();
    auto storeMetadataStatus = cache->storeMetadata(id, metadata);
    ASSERT_THAT (storeMetadataStatus, tempo_test::IsOk());

    auto generation2 = tempo_utils::UUID::randomUUID();
    lyric_build::ArtifactId linkId(generation2, hash, tempo_utils::UrlPath::fromString("/link"));

    lyric_build::MetadataWriter writer2;
    ASSERT_THAT (writer2.configure(), tempo_test::IsOk());
    writer2.putAttr(lyric_build::kLyricBuildGeneration, generation2.toString());
    auto createOverrideMetadataResult = writer2.toMetadata();
    ASSERT_THAT (createOverrideMetadataResult, tempo_test::IsResult());

    metadata = createOverrideMetadataResult.getResult();
    ASSERT_THAT (cache->linkArtifactOverridingMetadata(linkId, metadata, id), tempo_test::IsOk());

    auto loadMetadataResult = cache->loadMetadataFollowingLinks(linkId);
    ASSERT_THAT (loadMetadataResult, tempo_test::IsResult());
    metadata = loadMetadataResult.getResult();

    std::string value;
    ASSERT_THAT (metadata.parseAttr(lyric_build::kLyricBuildGeneration, value), tempo_test::IsOk());
    ASSERT_EQ (generation2.toString(), value);
}
