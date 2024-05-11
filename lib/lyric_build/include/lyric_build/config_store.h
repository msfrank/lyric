#ifndef LYRIC_BUILD_CONFIG_STORE_H
#define LYRIC_BUILD_CONFIG_STORE_H

#include <filesystem>

#include <lyric_build/build_types.h>
#include <tempo_config/abstract_config_parser.h>
#include <tempo_config/config_result.h>

namespace lyric_build {

    enum class NodeResolutionMode {
        IGNORE,
        INCLUDE,
    };

    class ConfigStore {

    public:
        ConfigStore();
        ConfigStore(
            const tempo_config::ConfigMap &config,
            const tempo_config::ConfigMap &vendorConfig);
        ConfigStore(const ConfigStore &other);

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

        ConfigStore merge(
            const tempo_config::ConfigMap &globalOverrides,
            const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainOverrides,
            const absl::flat_hash_map<TaskId,tempo_config::ConfigMap> &taskOverrides) const;

    private:
        struct Priv;
        std::shared_ptr<Priv> m_priv;

        explicit ConfigStore(std::shared_ptr<Priv> priv);
    };

    template <class T>
    tempo_utils::Status parse_config(
        T &dst,
        const tempo_config::AbstractConfigParser<T> &parser,
        const ConfigStore *store,
        std::string_view domain,
        std::string_view key)
    {
        TU_ASSERT (store != nullptr);
        auto node = store->resolveDomainNode(domain, key);
        return parser.parseValue(node, dst);
    }

    template <class T>
    tempo_utils::Status parse_config(
        T &dst,
        const tempo_config::AbstractConfigParser<T> &parser,
        const ConfigStore *store,
        const TaskId &task,
        std::string_view key)
    {
        TU_ASSERT (store != nullptr);
        auto node = store->resolveTaskNode(task, key);
        return parser.parseValue(node, dst);
    }
}

#endif // LYRIC_BUILD_CONFIG_STORE_H
