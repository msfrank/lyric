
#include <lyric_build/dependency_loader.h>

#include "archiver_tester.h"

ArchiverTester::ArchiverTester(const lyric_common::ModuleLocation &origin, lyric_test::LyricTester *tester)
    : m_origin(origin),
      m_tester(tester)
{
    TU_ASSERT (m_tester != nullptr);
    auto *runner = m_tester->getRunner();
    auto *builder = runner->getBuilder();
    auto tempRoot = builder->getTempRoot();
    m_tempDirectory = std::make_unique<lyric_build::TempDirectory>(tempRoot, "dependency-loader");
}

lyric_common::ModuleLocation
ArchiverTester::getOrigin() const
{
    return m_origin;
}

tempo_utils::Result<lyric_common::ModuleLocation>
ArchiverTester::writeModule(const std::string &code, const std::filesystem::path &path)
{
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, m_tester->writeModule(code, path));

    lyric_build::TaskId target("compile_module", moduleLocation.toString());
    m_taskIds.insert(std::move(target));

    return moduleLocation;
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::AbstractLoader>>
ArchiverTester::build()
{
    auto *builder = m_tester->getRunner()->getBuilder();
    auto cache = builder->getCache();

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, builder->computeTargets(m_taskIds));

    std::shared_ptr<lyric_runtime::AbstractLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(
        m_origin, targetComputationSet, cache, m_tempDirectory.get()));

    return loader;
}