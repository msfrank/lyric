#ifndef LYRIC_BUILD_TASK_SETTINGS_H
#define LYRIC_BUILD_TASK_SETTINGS_H

#include <filesystem>

#include <lyric_build/build_types.h>
#include <tempo_config/abstract_converter.h>
#include <tempo_config/config_result.h>

namespace lyric_build {

    enum class NodeResolutionMode {
        IGNORE,
        INCLUDE,
    };

    class TaskSettings {

    public:
        TaskSettings() = default;
        explicit TaskSettings(const tempo_config::ConfigMap &config);
        TaskSettings(
            const absl::flat_hash_map<std::string,tempo_config::ConfigNode> &globalSettings,
            const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainSettings,
            const absl::flat_hash_map<TaskId,tempo_config::ConfigMap> &taskSettings);
        TaskSettings(const TaskSettings &other);

        tempo_config::ConfigMap getConfig() const;

        tempo_config::ConfigMap getGlobalSection() const;
        tempo_config::ConfigNode getGlobalNode(std::string_view key) const;

        tempo_config::ConfigMap getDomainSection(std::string_view domain) const;
        tempo_config::ConfigNode getDomainNode(std::string_view domain, std::string_view key) const;
        std::vector<std::string> listDomainSections() const;

        tempo_config::ConfigMap getTaskSection(const TaskId &task) const;
        tempo_config::ConfigNode getTaskNode(const TaskId &task, std::string_view key) const;
        std::vector<TaskId> listTaskSections() const;

        tempo_config::ConfigNode resolveDomainNode(
            std::string_view domain,
            std::string_view key,
            NodeResolutionMode includeGlobal = NodeResolutionMode::INCLUDE) const;
        tempo_config::ConfigNode resolveTaskNode(
            const TaskId &task,
            std::string_view key,
            NodeResolutionMode includeDomain = NodeResolutionMode::INCLUDE,
            NodeResolutionMode includeGlobal = NodeResolutionMode::INCLUDE) const;

        TaskSettings merge(const TaskSettings &overrides) const;

    private:
        tempo_config::ConfigMap m_config;
    };

    template <class T>
    tempo_utils::Status parse_config(
        T &dst,
        const tempo_config::AbstractConverter<T> &converter,
        const TaskSettings *settings,
        std::string_view domain,
        std::string_view key)
    {
        TU_ASSERT (settings != nullptr);
        auto node = settings->resolveDomainNode(domain, key);
        return converter.convertValue(node, dst);
    }

    template <class T>
    tempo_utils::Status parse_config(
        T &dst,
        const tempo_config::AbstractConverter<T> &converter,
        const TaskSettings *settings,
        const TaskId &task,
        std::string_view key)
    {
        TU_ASSERT (settings != nullptr);
        auto node = settings->resolveTaskNode(task, key);
        return converter.convertValue(node, dst);
    }
}

#endif // LYRIC_BUILD_TASK_SETTINGS_H
