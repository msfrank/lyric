#ifndef ARCHIVER_TESTER_H
#define ARCHIVER_TESTER_H

#include <lyric_runtime/abstract_loader.h>
#include <lyric_test/lyric_tester.h>

class ArchiverTester {
public:
    ArchiverTester(const lyric_common::ModuleLocation &origin, lyric_test::LyricTester *tester);

    lyric_common::ModuleLocation getOrigin() const;

    tempo_utils::Result<lyric_common::ModuleLocation> writeModule(
        const std::string &code,
        const std::filesystem::path &path);

    tempo_utils::Result<std::shared_ptr<lyric_runtime::AbstractLoader>> build();
private:
    lyric_common::ModuleLocation m_origin;
    lyric_test::LyricTester *m_tester;
    std::unique_ptr<lyric_build::TempDirectory> m_tempDirectory;
    absl::flat_hash_set<lyric_build::TaskId> m_taskIds;
};

#endif // ARCHIVER_TESTER_H
