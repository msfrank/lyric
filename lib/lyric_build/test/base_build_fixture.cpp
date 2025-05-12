
#include "base_build_fixture.h"

#include <tempo_utils/file_writer.h>
#include <tempo_utils/tempdir_maker.h>
#include <tempo_utils/tempfile_maker.h>

#include "lyric_build/memory_cache.h"

void
BaseBuildFixture::SetUp()
{
    m_generation = tempo_utils::UUID::randomUUID();
    m_recorder = tempo_tracing::TraceRecorder::create();
    m_span = m_recorder->makeSpan();

    tempo_utils::TempdirMaker tempdirMaker(std::filesystem::current_path(), "tester.XXXXXXXX");
    TU_RAISE_IF_NOT_OK (tempdirMaker.getStatus());
    m_testerDirectory = tempdirMaker.getTempdir();

    TU_LOG_INFO << "created tester directory " << m_testerDirectory;

    TU_ASSIGN_OR_RAISE (m_vfs, lyric_build::LocalFilesystem::create(m_testerDirectory));

    auto buildgen = lyric_build::BuildGeneration::create();
    auto cache = std::make_shared<lyric_build::MemoryCache>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto packageLoader = std::make_shared<lyric_packaging::PackageLoader>();
    m_state = std::make_unique<lyric_build::BuildState>(buildgen,
        std::static_pointer_cast<lyric_build::AbstractCache>(cache),
        bootstrapLoader,
        packageLoader,
        std::shared_ptr<lyric_runtime::AbstractLoader>{},
        std::shared_ptr<lyric_importer::ModuleCache>{},
        m_vfs,
        m_testerDirectory);

    TU_RAISE_IF_NOT_OK (configure());
}

std::filesystem::path
BaseBuildFixture::writeNamedFile(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &path,
    std::string_view content)
{
    auto relativePath = baseDir / path;
    auto absolutePath = m_testerDirectory / relativePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec) && ec) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create directory {}: {}", parentPath.string(), ec.message());
        throw tempo_utils::StatusException(status);
    }

    tempo_utils::FileWriter writer(absolutePath.string(), content, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    if (!writer.isValid()) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to write file {}", absolutePath.string());
        throw tempo_utils::StatusException(status);
    }

    return relativePath;
}

std::filesystem::path
BaseBuildFixture::writeTempFile(
    const std::filesystem::path &baseDir,
    const std::filesystem::path &templatePath,
    std::string_view content)
{
    auto absolutePath = m_testerDirectory / baseDir / templatePath;
    auto parentPath = absolutePath.parent_path();
    std::error_code ec;
    if (!std::filesystem::create_directories(parentPath, ec) && ec) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create directory {}: {}", parentPath.string(), ec.message());
        throw tempo_utils::StatusException(status);
    }

    tempo_utils::TempfileMaker tempfileMaker(absolutePath.parent_path().string(),
        absolutePath.filename().string(), content);
    if (!tempfileMaker.isValid()) {
        auto status = tempo_utils::GenericStatus::forCondition(tempo_utils::GenericCondition::kInternalViolation,
            "failed to create temp file {}", absolutePath.string());
        throw tempo_utils::StatusException(status);
    }

    return std::filesystem::relative(tempfileMaker.getTempfile(), m_testerDirectory);
}

tempo_utils::Status
BaseBuildFixture::configure()
{
    m_config = std::make_unique<lyric_build::ConfigStore>(tempo_config::ConfigMap{}, tempo_config::ConfigMap{});
    return {};
}
