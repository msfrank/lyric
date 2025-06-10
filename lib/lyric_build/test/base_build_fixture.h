#ifndef LYRIC_BUILD_BASE_BUILD_FIXTURE_H
#define LYRIC_BUILD_BASE_BUILD_FIXTURE_H

#include <gtest/gtest.h>

#include <lyric_assembler/object_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/local_filesystem.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_utils/uuid.h>

#include "lyric_build/build_state.h"

class BaseBuildFixture : public ::testing::Test {
protected:
    tempo_utils::UUID m_generation;
    std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;
    std::shared_ptr<tempo_tracing::TraceSpan> m_span;
    std::filesystem::path m_testerDirectory;
    std::unique_ptr<lyric_build::TaskSettings> m_config;
    std::shared_ptr<lyric_build::LocalFilesystem> m_vfs;
    std::unique_ptr<lyric_build::BuildState> m_state;

    std::filesystem::path writeNamedFile(
        const std::filesystem::path &baseDir,
        const std::filesystem::path &path,
        std::string_view content);

    std::filesystem::path writeTempFile(
        const std::filesystem::path &baseDir,
        const std::filesystem::path &templatePath,
        std::string_view content);

    virtual tempo_utils::Status configure();

    void SetUp() override;
};

#endif // LYRIC_BUILD_BASE_BUILD_FIXTURE_H
