
#include <lyric_build/base_task.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/internal/analyze_module_task.h>
#include <lyric_build/internal/archive_task.h>
#include <lyric_build/internal/build_task.h>
#include <lyric_build/internal/compile_task.h>
#include <lyric_build/internal/compile_module_task.h>
#include <lyric_build/internal/orchestrate_task.h>
#include <lyric_build/internal/package_task.h>
#include <lyric_build/internal/parse_module_task.h>
#include <lyric_build/internal/rewrite_module_task.h>
#include <lyric_build/internal/symbolize_module_task.h>
#include <lyric_build/internal/test_task.h>
#include <lyric_build/task_registry.h>

struct Task {
    const char *name;
    lyric_build::BaseTask* (*func)(
        const tempo_utils::UUID &generation,
        const lyric_build::TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
};

static const Task tasks[] = {
    {"analyze_module",      lyric_build::internal::new_analyze_module_task},
    {"archive",             lyric_build::internal::new_archive_task},
    {"build",               lyric_build::internal::new_build_task},
    {"compile",             lyric_build::internal::new_compile_task},
    {"compile_module",      lyric_build::internal::new_compile_module_task},
    {"orchestrate",         lyric_build::internal::new_orchestrate_task},
    {"package",             lyric_build::internal::new_package_task},
    {"parse_module",        lyric_build::internal::new_parse_module_task},
    {"rewrite_module",      lyric_build::internal::new_rewrite_module_task},
    {"symbolize_module",    lyric_build::internal::new_symbolize_module_task},
    {"test",                lyric_build::internal::new_test_task},
    {nullptr, nullptr},     // sentinel value, must be last
};

lyric_build::TaskRegistry::TaskRegistry()
{
    for (int i = 0; tasks[i].name != nullptr; i++) {
        auto &task = tasks[i];
        m_makeTaskFuncs[task.name] = task.func;
    }
}

tempo_utils::Status
lyric_build::TaskRegistry::registerTaskDomain(std::string_view domain, MakeTaskFunc func)
{
    if (m_makeTaskFuncs.contains(domain))
        return BuildStatus::forCondition(
            BuildCondition::kBuildInvariant, "task domain '{}' is already registered", domain);
    m_makeTaskFuncs[domain] = func;
    return {};
}

tempo_utils::Result<lyric_build::BaseTask *>
lyric_build::TaskRegistry::makeTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    const auto domain = key.getDomain();
    auto entry = m_makeTaskFuncs.find(domain);
    if (entry == m_makeTaskFuncs.cend())
        return BuildStatus::forCondition(
            BuildCondition::kInvalidConfiguration, "unknown task domain '{}'", domain);
    BaseTask *task = entry->second(generation, key, span);
    if (task == nullptr)
        return BuildStatus::forCondition(
            BuildCondition::kBuildInvariant, "invalid task domain '{}'", domain);
    return task;
}
