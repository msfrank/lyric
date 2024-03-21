
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
    {"symbolize_module",    lyric_build::internal::new_symbolize_module_task},
    {"test",                lyric_build::internal::new_test_task},
    {nullptr, nullptr}, // sentinel value, must be last
};

lyric_build::TaskRegistry::TaskRegistry(const ConfigStore &config)
    : m_config(config)
{
}

tempo_utils::Result<lyric_build::BaseTask *>
lyric_build::TaskRegistry::makeTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    const auto domain = key.getDomain();

    BaseTask *task = nullptr;
    for (int i = 0; tasks[i].name != nullptr; i++) {
        if (tasks[i].name == domain) {
            task = tasks[i].func(generation, key, span);
            break;
        }
    }

    if (task == nullptr)
        return BuildStatus::forCondition(
            BuildCondition::kInvalidConfiguration, "invalid task domain {}", key.getDomain());
    return task;
}
