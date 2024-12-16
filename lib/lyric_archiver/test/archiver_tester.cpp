
#include <lyric_build/dependency_loader.h>

#include "archiver_tester.h"

ArchiverTester::ArchiverTester(lyric_test::LyricTester *tester)
    : m_tester(tester)
{
    TU_ASSERT (m_tester != nullptr);
}

tempo_utils::Result<lyric_common::ModuleLocation>
ArchiverTester::writeModule(const std::string &code, const std::filesystem::path &path)
{
    std::filesystem::path sourcePath;
    TU_ASSIGN_OR_RETURN (sourcePath, m_tester->writeModule(code, path));

    lyric_build::TaskId target("compile_module", sourcePath);
    m_taskIds.insert(std::move(target));

    std::filesystem::path modulePath = "/";
    modulePath /= sourcePath;
    modulePath.replace_extension();
    return lyric_common::ModuleLocation(modulePath.string());
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::AbstractLoader>>
ArchiverTester::build()
{
    auto *builder = m_tester->getRunner()->getBuilder();
    auto cache = builder->getCache();

    lyric_build::TargetComputationSet targetComputationSet;
    TU_ASSIGN_OR_RETURN (targetComputationSet, builder->computeTargets(m_taskIds, {}, {}, {}));

    std::shared_ptr<lyric_runtime::AbstractLoader> loader;
    TU_ASSIGN_OR_RETURN (loader, lyric_build::DependencyLoader::create(targetComputationSet, cache));

    return loader;
}