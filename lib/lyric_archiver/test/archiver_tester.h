#ifndef ARCHIVER_TESTER_H
#define ARCHIVER_TESTER_H

#include <lyric_runtime/abstract_loader.h>
#include <lyric_test/lyric_tester.h>

class ArchiverTester {
public:
    explicit ArchiverTester(lyric_test::LyricTester *tester);

    tempo_utils::Result<lyric_common::ModuleLocation> writeModule(
        const std::string &code,
        const std::filesystem::path &path);

    tempo_utils::Result<std::shared_ptr<lyric_runtime::AbstractLoader>> build();
private:
    lyric_test::LyricTester *m_tester;
    absl::flat_hash_set<lyric_build::TaskId> m_taskIds;
};

#endif // ARCHIVER_TESTER_H
