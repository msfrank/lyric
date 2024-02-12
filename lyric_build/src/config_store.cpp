
#include <lyric_build/config_store.h>
#include <lyric_build/build_types.h>
#include <tempo_config/merge_map.h>

struct lyric_build::ConfigStore::Priv {
    tempo_config::ConfigMap config;
    tempo_config::ConfigMap vendorConfig;
    Priv() {};
    Priv(const tempo_config::ConfigMap &config, const tempo_config::ConfigMap &vendorConfig)
        : config(config),
          vendorConfig(vendorConfig) {}
};

/**
 * Construct an empty ConfigStore.
 */
lyric_build::ConfigStore::ConfigStore()
    : m_priv(std::make_shared<Priv>())
{
}

/**
 * Construct a ConfigStore using the specified config and vendor config.
 *
 * @param config The build config.
 * @param vendorConfig The vendor config.
 */
lyric_build::ConfigStore::ConfigStore(
    const tempo_config::ConfigMap &config,
    const tempo_config::ConfigMap &vendorConfig)
{
    m_priv = std::make_shared<Priv>(config, vendorConfig);
}

lyric_build::ConfigStore::ConfigStore(std::shared_ptr<Priv> priv)
    : m_priv(priv)
{
    TU_ASSERT (m_priv != nullptr);
}

/**
 * Copy construct a ConfigStore from the given ConfigStore.
 *
 * @param other ConfigStore.
 */
lyric_build::ConfigStore::ConfigStore(const lyric_build::ConfigStore &other)
    : m_priv(other.m_priv)
{
}

/**
 * Return the config map for the global section.
 *
 * @return The config map.
 */
tempo_config::ConfigMap
lyric_build::ConfigStore::getGlobalSection() const
{
    return m_priv->config.mapAt("global").toMap();
}

/**
 * Return the config node corresponding to the specified key in the global section.
 *
 * @param key The config key.
 * @return The config node.
 */
tempo_config::ConfigNode
lyric_build::ConfigStore::getGlobalNode(std::string_view key) const
{
    return getGlobalSection().mapAt(key);
}

/**
 * Return the config map corresponding to the specified domain in the domains section.
 * @param domain The domain key.
 * @return The config map.
 */
tempo_config::ConfigMap
lyric_build::ConfigStore::getDomainSection(std::string_view domain) const
{
    return m_priv->config
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
lyric_build::ConfigStore::getDomainNode(std::string_view domain, std::string_view key) const
{
    return getDomainSection(domain).mapAt(key);
}

/**
 * Return a list containing all of the keys in the domains section.
 *
 * @return The list of keys.
 */
std::vector<std::string>
lyric_build::ConfigStore::listDomainSections() const
{
    auto domainsMap = m_priv->config.mapAt("domains").toMap();
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
lyric_build::ConfigStore::getTaskSection(const TaskId &key) const
{
    return m_priv->config
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
lyric_build::ConfigStore::getTaskNode(const TaskId &task, std::string_view key) const
{
    return getTaskSection(task).mapAt(key);
}

/**
 * Return a list containing all of the keys in the tasks section.
 * @return
 */
std::vector<lyric_build::TaskId>
lyric_build::ConfigStore::listTaskSections() const
{
    auto tasksMap = m_priv->config.mapAt("tasks").toMap();
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
lyric_build::ConfigStore::resolveDomainNode(
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
lyric_build::ConfigStore::resolveTaskNode(
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

static tempo_config::ConfigMap
generate_overrides(
    const tempo_config::ConfigMap &global,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domains,
    const absl::flat_hash_map<lyric_build::TaskId,tempo_config::ConfigMap> &tasks)
{
    absl::flat_hash_map<std::string,tempo_config::ConfigNode> domainNodes;
    for (auto iterator : domains) {
        domainNodes[iterator.first] = iterator.second;
    }
    tempo_config::ConfigMap domainsMap(std::move(domainNodes));

    absl::flat_hash_map<std::string,tempo_config::ConfigNode> tasksNodes;
    for (auto iterator : tasks) {
        tasksNodes[iterator.first.toString()] = iterator.second;
    }
    tempo_config::ConfigMap tasksMap(std::move(tasksNodes));

    return tempo_config::ConfigMap{
        {"global", global},
        {"domains", domainsMap},
        {"tasks", tasksMap},
    };
}

/**
 * Merge the ConfigStore with the specified config overrides and return a new ConfigStore.
 *
 * @param globalOverrides The config map containing global overrides.
 * @param domainOverrides The config map containing domain overrides.
 * @param taskOverrides The config map containing task overrides.
 * @return The config store.
 */
lyric_build::ConfigStore
lyric_build::ConfigStore::merge(
    const tempo_config::ConfigMap &globalOverrides,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainOverrides,
    const absl::flat_hash_map<TaskId,tempo_config::ConfigMap> &taskOverrides) const
{
    auto overrides = generate_overrides(globalOverrides, domainOverrides, taskOverrides);
    auto priv = std::make_shared<Priv>(
        tempo_config::merge_map(m_priv->config, overrides),
        m_priv->vendorConfig);
    return ConfigStore(priv);
}