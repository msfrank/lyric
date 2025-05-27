
#include <lyric_build/base_task.h>
#include <lyric_build/build_types.h>
#include <lyric_build/internal/analyze_module_task.h>
#include <lyric_build/internal/archive_task.h>
#include <lyric_build/internal/build_task.h>
#include <lyric_build/internal/compile_task.h>
#include <lyric_build/internal/compile_module_task.h>
#include <lyric_build/internal/compile_plugin_task.h>
#include <lyric_build/internal/fetch_external_file_task.h>
#include <lyric_build/internal/orchestrate_task.h>
#include <lyric_build/internal/parse_module_task.h>
#include <lyric_build/internal/provide_module_task.h>
#include <lyric_build/internal/provide_plugin_task.h>
#include <lyric_build/internal/rewrite_module_task.h>
#include <lyric_build/internal/symbolize_module_task.h>
#include <lyric_build/task_registry.h>

struct TaskDomain {
    const char *name;
    lyric_build::BaseTask* (*func)(
        const tempo_utils::UUID &generation,
        const lyric_build::TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
};

static const TaskDomain predefinedTaskDomains[] = {
    {"analyze_module",      lyric_build::internal::new_analyze_module_task},
    {"archive",             lyric_build::internal::new_archive_task},
    {"build",               lyric_build::internal::new_build_task},
    {"compile",             lyric_build::internal::new_compile_task},
    {"compile_module",      lyric_build::internal::new_compile_module_task},
    {"compile_plugin",      lyric_build::internal::new_compile_plugin_task},
    {"fetch_external_file", lyric_build::internal::new_fetch_external_file_task},
    {"orchestrate",         lyric_build::internal::new_orchestrate_task},
    {"parse_module",        lyric_build::internal::new_parse_module_task},
    {"provide_module",      lyric_build::internal::new_provide_module_task},
    {"provide_plugin",      lyric_build::internal::new_provide_plugin_task},
    {"rewrite_module",      lyric_build::internal::new_rewrite_module_task},
    {"symbolize_module",    lyric_build::internal::new_symbolize_module_task},
    {nullptr, nullptr},     // sentinel value, must be last
};

lyric_build::TaskRegistry::TaskRegistry(bool excludePredefinedDomains)
    : m_isSealed(false)
{
    if (!excludePredefinedDomains) {
        for (int i = 0; predefinedTaskDomains[i].name != nullptr; i++) {
            auto &task = predefinedTaskDomains[i];
            m_makeTaskFuncs[task.name] = task.func;
        }
    }
}

tempo_utils::Status
lyric_build::TaskRegistry::registerTaskDomain(std::string_view domain, MakeTaskFunc func)
{
    TU_ASSERT (func != nullptr);
    if (domain.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task domain '{}'", domain);
    if (m_isSealed)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "cannot mutate sealed task registry");
    if (m_makeTaskFuncs.contains(domain))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "task domain '{}' is already registered", domain);
    m_makeTaskFuncs[domain] = func;
    return {};
}

tempo_utils::Status
lyric_build::TaskRegistry::replaceTaskDomain(std::string_view domain, MakeTaskFunc func)
{
    TU_ASSERT (func != nullptr);
    if (domain.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task domain '{}'", domain);
    if (m_isSealed)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "cannot mutate sealed task registry");
    m_makeTaskFuncs[domain] = func;
    return {};
}

tempo_utils::Status
lyric_build::TaskRegistry::deregisterTaskDomain(std::string_view domain)
{
    if (domain.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid task domain '{}'", domain);
    if (m_isSealed)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "cannot mutate sealed task registry");
    if (!m_makeTaskFuncs.contains(domain))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "task domain '{}' is not registered", domain);
    m_makeTaskFuncs.erase(domain);
    return {};
}

void
lyric_build::TaskRegistry::sealRegistry()
{
    m_isSealed = true;
}

tempo_utils::Result<lyric_build::BaseTask *>
lyric_build::TaskRegistry::makeTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span) const
{
    if (!m_isSealed)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "task registry must be sealed to make tasks");
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
