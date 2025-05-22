
#include <lyric_build/task_settings.h>
#include <lyric_build/build_types.h>
#include <tempo_config/merge_map.h>

static tempo_config::ConfigMap
generate_config(
    const absl::flat_hash_map<std::string,tempo_config::ConfigNode> &global,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domains,
    const absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> &tasks)
{
    tempo_config::ConfigMap globalMap(global);

    absl::flat_hash_map<std::string,tempo_config::ConfigNode> domainNodes;
    for (const auto &entry : domains) {
        domainNodes[entry.first] = entry.second;
    }
    tempo_config::ConfigMap domainsMap(std::move(domainNodes));

    absl::flat_hash_map<std::string,tempo_config::ConfigNode> tasksNodes;
    for (const auto &entry : tasks) {
        tasksNodes[entry.first.toString()] = entry.second;
    }
    tempo_config::ConfigMap tasksMap(std::move(tasksNodes));

    return tempo_config::ConfigMap{
        {"global", globalMap},
       {"domains", domainsMap},
       {"tasks", tasksMap},
    };
}

/**
 * Construct an empty TaskSettings.
 */
lyric_build::TaskSettings::TaskSettings(
    const absl::flat_hash_map<std::string,tempo_config::ConfigNode> &globalSettings,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainSettings,
    const absl::flat_hash_map<TaskId,tempo_config::ConfigMap> &taskSettings)
{
    m_config = generate_config(globalSettings, domainSettings, taskSettings);
}

/**
 * Construct a TaskSettings using the specified config and vendor config.
 *
 * @param config The build config.
 * @param vendorConfig The vendor config.
 */
lyric_build::TaskSettings::TaskSettings(const tempo_config::ConfigMap &config)
    : m_config(config)
{
}

/**
 * Copy construct a TaskSettings from the given TaskSettings.
 *
 * @param other ConfigStore.
 */
lyric_build::TaskSettings::TaskSettings(const TaskSettings &other)
    : m_config(other.m_config)
{
}

/**
 * Return the config.
 *
 * @return The config.
 */
tempo_config::ConfigMap
lyric_build::TaskSettings::getConfig() const
{
    return m_config;
}

/**
 * Return the config map for the global section.
 *
 * @return The config map.
 */
tempo_config::ConfigMap
lyric_build::TaskSettings::getGlobalSection() const
{
    return m_config.mapAt("global").toMap();
}

/**
 * Return the config node corresponding to the specified key in the global section.
 *
 * @param key The config key.
 * @return The config node.
 */
tempo_config::ConfigNode
lyric_build::TaskSettings::getGlobalNode(std::string_view key) const
{
    return getGlobalSection().mapAt(key);
}

/**
 * Return the config map corresponding to the specified domain in the domains section.
 * @param domain The domain key.
 * @return The config map.
 */
tempo_config::ConfigMap
lyric_build::TaskSettings::getDomainSection(std::string_view domain) const
{
    return m_config
        .mapAt("domains").toMap()
        .mapAt(domain).toMap();
}

/**
 * Return the config node corresponding to the specified key in the specified domain in the domains section.
 *
 * @param domain The domain key.
 * @param key The config key.
 * @return The config node.
 */
tempo_config::ConfigNode
lyric_build::TaskSettings::getDomainNode(std::string_view domain, std::string_view key) const
{
    return getDomainSection(domain).mapAt(key);
}

/**
 * Return a list containing all of the keys in the domains section.
 *
 * @return The list of keys.
 */
std::vector<std::string>
lyric_build::TaskSettings::listDomainSections() const
{
    auto domainsMap = m_config.mapAt("domains").toMap();
    std::vector<std::string> sections;
    for (auto iterator = domainsMap.mapBegin(); iterator != domainsMap.mapEnd(); iterator++) {
        sections.push_back(iterator->first);
    }
    return sections;
}

/**
 * Return the config map corresponding to the specified task in the tasks section.
 *
 * @param key The task key.
 * @return The config map.
 */
tempo_config::ConfigMap
lyric_build::TaskSettings::getTaskSection(const TaskId &key) const
{
    return m_config
        .mapAt("tasks").toMap()
        .mapAt(key.toString()).toMap();
}

/**
 * Return the config node corresponding to the specified key in the specified task in the tasks section.
 *
 * @param task The task key.
 * @param key The config key.
 * @return The config node.
 */
tempo_config::ConfigNode
lyric_build::TaskSettings::getTaskNode(const TaskId &task, std::string_view key) const
{
    return getTaskSection(task).mapAt(key);
}

/**
 * Return a list containing all of the keys in the tasks section.
 * @return
 */
std::vector<lyric_build::TaskId>
lyric_build::TaskSettings::listTaskSections() const
{
    auto tasksMap = m_config.mapAt("tasks").toMap();
    std::vector<TaskId> sections;
    for (auto iterator = tasksMap.mapBegin(); iterator != tasksMap.mapEnd(); iterator++) {
        sections.push_back(TaskId::fromString(iterator->first));
    }
    return sections;
}

/**
 * Find the ConfigNode for the given domain and key.
 *
 * @param domain The domain.
 * @param key The key.
 * @param includeGlobal if INCLUDE, then check the global section if the key is not present in the domain.
 * @return The matching ConfigNode if present, otherwise an empty ConfigNode
 */
tempo_config::ConfigNode
lyric_build::TaskSettings::resolveDomainNode(
    std::string_view domain,
    std::string_view key,
    NodeResolutionMode includeGlobal) const
{
    auto node = getDomainNode(domain, key);
    if (!node.isNil())
        return node;
    if (includeGlobal == NodeResolutionMode::INCLUDE)
        return getGlobalNode(key);
    return {};
}

/**
 * Find the ConfigNode for the given task and key.
 *
 * @param domain The domain.
 * @param key The key.
 * @param includeDomain if INCLUDE, then check the domain section for the key if not present in the task.
 * @param includeGlobal if INCLUDE, then check the global section for the key if not present in the domain.
 * @return The matching ConfigNode if present, otherwise an empty ConfigNode
 */
tempo_config::ConfigNode
lyric_build::TaskSettings::resolveTaskNode(
    const TaskId &task,
    std::string_view key,
    NodeResolutionMode includeDomain,
    NodeResolutionMode includeGlobal) const
{
    auto node = getTaskNode(task, key);
    if (!node.isNil())
        return node;
    if (includeDomain == NodeResolutionMode::INCLUDE) {
        node = getDomainNode(task.getDomain(), key);
        if (!node.isNil())
            return node;
    }
    if (includeGlobal == NodeResolutionMode::INCLUDE)
        return getGlobalNode(key);
    return {};
}


/**
 * Merge the TaskSettings with the specified config overrides and return a new TaskSettings.
 *
 * @param globalOverrides The config map containing global overrides.
 * @param domainOverrides The config map containing domain overrides.
 * @param taskOverrides The config map containing task overrides.
 * @return The config store.
 */
lyric_build::TaskSettings
lyric_build::TaskSettings::merge(const TaskSettings &overrides) const
{
    auto merged = tempo_config::merge_map(m_config, overrides.getConfig());
    return TaskSettings(merged);
}