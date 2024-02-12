#include <gtest/gtest.h>

#include <lyric_packaging/package_writer.h>
#include <tempo_utils/tempdir_maker.h>

//TEST(PackageWriter, TestWritePackage)
//{
//    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "test_PackageWriter.XXXXXXXX");
//    ASSERT_TRUE (tempdirMaker.isValid());
//    lyric_packaging::PackageWriter writer;
//    writer.putFile(lyric_packaging::EntryPath::fromString("/file.txt"), "hello, world!");
//    auto packagePath = tempdirMaker.getTempdir() / "package.lyz";
//    auto status = writer.writePackage(packagePath);
//    ASSERT_TRUE (status.isOk());
//    ASSERT_TRUE (exists(packagePath));
//    ASSERT_TRUE (remove(packagePath));
//}