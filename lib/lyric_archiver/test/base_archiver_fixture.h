#ifndef BASE_ARCHIVER_FIXTURE_H
#define BASE_ARCHIVER_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_archiver/lyric_archiver.h>
#include <lyric_runtime/static_loader.h>

#include "archiver_tester.h"

class BaseArchiverFixture : public ::testing::Test {
protected:
    BaseArchiverFixture();

    tempo_utils::Status configure();

    tempo_utils::Result<lyric_common::ModuleLocation> writeModule(
        const std::string &code,
        const std::filesystem::path &path);

    tempo_utils::Status prepare();

    tempo_utils::Status importModule(const lyric_common::ModuleLocation &location);

    tempo_utils::Result<lyric_common::SymbolUrl> archiveSymbol(
        const lyric_common::ModuleLocation &location,
        const std::string &identifier);

    tempo_utils::Result<lyric_assembler::BindingSymbol *> declareBinding(
        const std::string &name,
        lyric_object::AccessType access,
        const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

    tempo_utils::Status build();

    tempo_utils::Result<lyric_test::RunModule> runCode(const std::string &code);

private:
    std::shared_ptr<lyric_runtime::StaticLoader> m_staticLoader;
    lyric_test::TesterOptions m_testerOptions;
    std::unique_ptr<lyric_test::LyricTester> m_tester;
    std::unique_ptr<ArchiverTester> m_archiverTester;
    std::unique_ptr<lyric_archiver::LyricArchiver> m_archiver;
    lyric_common::ModuleLocation m_archiveLocation;
};


#endif // BASE_ARCHIVER_FIXTURE_H
