#include <gtest/gtest.h>

#include <lyric_build/sha256_hash.h>

TEST(Sha256Hash, GenerateHashValue)
{
    auto hash = lyric_build::Sha256Hash::hash("hello, world!");
    ASSERT_FALSE (hash.empty());
}