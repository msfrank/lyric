#include <gtest/gtest.h>

#include <lyric_packaging/package_reader.h>
#include <lyric_packaging/package_writer.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/tempfile_maker.h>

TEST(PackageReader, TestReadFileFromPackage)
{
    lyric_packaging::PackageWriter writer;
    ASSERT_TRUE (writer.configure().isOk());

    auto path = lyric_packaging::EntryPath::fromString("/file.txt");
    auto src = tempo_utils::MemoryBytes::copy("hello, world!");
    writer.putFile(path, src);
    auto toBytesResult = writer.toBytes();
    ASSERT_TRUE (toBytesResult.isResult());
    auto packageBytes = toBytesResult.getResult();

    auto createReaderResult = lyric_packaging::PackageReader::create(packageBytes);
    ASSERT_TRUE (createReaderResult.isResult());

    auto reader = createReaderResult.getResult();
    ASSERT_TRUE (reader->isValid());
    ASSERT_EQ (13, reader->getFileSize(path));
    auto fileContents = reader->getFileContents(path);
    ASSERT_FALSE (fileContents.isEmpty());
    std::string contents((const char *) fileContents.getData(), fileContents.getSize());
    ASSERT_EQ ("hello, world!", contents);
}

TEST(PackageReader, TestOpenPackageFromFile)
{
    lyric_packaging::PackageWriter writer;
    auto src = tempo_utils::MemoryBytes::copy("hello, world!");
    writer.putFile(lyric_packaging::EntryPath::fromString("/file.txt"), src);
    auto toBytesResult = writer.toBytes();
    ASSERT_TRUE (toBytesResult.isResult());
    auto packageBytes = toBytesResult.getResult();

    tempo_utils::TempfileMaker tempfileMaker(std::filesystem::current_path(), "package.XXXXXXXX",
        std::string_view((const char *) packageBytes->getData(), packageBytes->getSize()));
    ASSERT_TRUE (tempfileMaker.isValid());
    auto packagePath = tempfileMaker.getTempfile();

    auto createReaderResult = lyric_packaging::PackageReader::open(packagePath);
    ASSERT_TRUE (createReaderResult.isResult());

    auto reader = createReaderResult.getResult();
    ASSERT_TRUE (reader->isValid());
    ASSERT_TRUE (remove(packagePath));
}
