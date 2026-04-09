
#include <absl/strings/escaping.h>
#include <absl/strings/str_join.h>
#include <absl/strings/substitute.h>

#include <lyric_build/build_types.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_stream.h>

lyric_build::BuildGeneration::BuildGeneration()
{
}

lyric_build::BuildGeneration::BuildGeneration(const tempo_utils::UUID &uuid)
    : m_uuid(uuid)
{
    TU_ASSERT (m_uuid.isValid());
}

lyric_build::BuildGeneration::BuildGeneration(const BuildGeneration &other)
    : m_uuid(other.m_uuid)
{
}

bool
lyric_build::BuildGeneration::isValid() const
{
    return m_uuid.isValid() && !m_uuid.isNil();
}

std::vector<tu_uint8>
lyric_build::BuildGeneration::toBytes() const
{
    return m_uuid.toBytes();
}
std::string
lyric_build::BuildGeneration::toString() const
{
    return m_uuid.toCompactString();
}

bool
lyric_build::BuildGeneration::operator==(const BuildGeneration &other) const
{
    return m_uuid.compare(other.m_uuid) == 0;
}

bool
lyric_build::BuildGeneration::operator!=(const BuildGeneration &other) const
{
    return m_uuid.compare(other.m_uuid) != 0;
}

bool
lyric_build::BuildGeneration::operator<(const BuildGeneration &other) const
{
    return m_uuid.compare(other.m_uuid) < 0;
}

lyric_build::BuildGeneration
lyric_build::BuildGeneration::create()
{
    auto uuid = tempo_utils::UUID::randomUUID();
    return BuildGeneration(uuid);
}

lyric_build::BuildGeneration
lyric_build::BuildGeneration::parse(std::string_view s)
{
    auto uuid = tempo_utils::UUID::parse(s);
    if (uuid.isValid())
        return BuildGeneration(uuid);
    return {};
}

lyric_build::TaskHash::TaskHash(std::vector<tu_uint8> taskHash)
    : m_hash(std::make_shared<std::vector<tu_uint8>>(std::move(taskHash)))
{
}

lyric_build::TaskHash::TaskHash(const TaskHash &other)
    : m_hash(other.m_hash)
{
}

bool
lyric_build::TaskHash::isValid()
{
    return m_hash != nullptr;
}

std::span<tu_uint8>
lyric_build::TaskHash::bytesView() const
{
    if (m_hash == nullptr)
        return {};
    return std::span(m_hash->data(), m_hash->size());
}

std::vector<tu_uint8>
lyric_build::TaskHash::toBytes() const
{
    if (m_hash == nullptr)
        return {};
    return *m_hash;
}

std::string
lyric_build::TaskHash::toString() const
{
    if (m_hash == nullptr)
        return {};
    std::string_view sv((const char *) m_hash->data(), m_hash->size());
    return absl::BytesToHexString(sv);
}

int
lyric_build::TaskHash::compare(const TaskHash &other) const
{
    if (m_hash) {
        if (other.m_hash == nullptr)
            return 1;
        auto size = std::min(m_hash->size(), other.m_hash->size());
        return memcmp(m_hash->data(), other.m_hash->data(), size);
    }
    return other.m_hash? -1 : 0;
}

bool
lyric_build::TaskHash::operator==(const TaskHash &other) const
{
    return compare(other) == 0;
}

bool
lyric_build::TaskHash::operator!=(const TaskHash &other) const
{
    return compare(other) != 0;
}

bool
lyric_build::TaskHash::operator<(const TaskHash &other) const
{
    return compare(other) < 0;
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::string &id)
    : m_priv(std::make_shared<Priv>(domain, id))
{
    TU_ASSERT (!m_priv->domain.empty());
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::vector<std::string> &parts)
    : TaskId(domain, absl::StrJoin(parts, "/"))
{
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::filesystem::path &path)
    : TaskId(domain, path.string())
{
}

lyric_build::TaskId::TaskId(const std::string &domain, const tempo_utils::UrlPath &urlPath)
    : TaskId(domain, urlPath.toString())
{
}

lyric_build::TaskId::TaskId(const TaskId &other)
    : m_priv(other.m_priv)
{
}

bool
lyric_build::TaskId::isValid() const
{
    return m_priv != nullptr;
}

std::string
lyric_build::TaskId::getDomain() const
{
    return m_priv? m_priv->domain : "";
}

std::string
lyric_build::TaskId::getId() const
{
    return m_priv? m_priv->id : "";
}

std::string
lyric_build::TaskId::toString() const
{
    if (m_priv != nullptr)
        return absl::StrCat(m_priv->domain, ":", m_priv->id);
    return {};
}

int
lyric_build::TaskId::compare(const TaskId &other) const
{
    if (m_priv) {
        if (other.m_priv == nullptr)
            return 1;
        auto domaincmp = m_priv->domain.compare(other.m_priv->domain);
        if (domaincmp != 0)
            return domaincmp;
        return m_priv->id.compare(other.m_priv->id);
    }
    return other.m_priv? -1 : 0;
}

bool
lyric_build::TaskId::operator==(const TaskId &other) const
{
    return compare(other) == 0;
}

bool
lyric_build::TaskId::operator!=(const TaskId &other) const
{
    return compare(other) != 0;
}

bool
lyric_build::TaskId::operator<(const TaskId &other) const
{
    return compare(other) < 0;
}

lyric_build::TaskId
lyric_build::TaskId::fromString(const std::string &s)
{
    auto split = s.find_first_of(':');
    if (split == std::string::npos)
        return lyric_build::TaskId(s, std::string(""));
    auto domain = s.substr(0, split);
    auto id = s.substr(split + 1);
    return TaskId(domain, id);
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const TaskId &taskId)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "TaskId(domain=" << taskId.getDomain()
        << ", id=" << taskId.getId()
        <<  ")";
    return std::move(message);
}

lyric_build::TaskKey::TaskKey(
    const std::string &domain,
    const std::string &id,
    const tempo_config::ConfigMap &params)
    : m_priv(std::make_shared<Priv>(domain, id, params))
{
    TU_ASSERT (!m_priv->domain.empty());
}

lyric_build::TaskKey::TaskKey(
    const std::string &domain,
    const std::vector<std::string> &parts,
    const tempo_config::ConfigMap &params)
    : TaskKey(domain, absl::StrJoin(parts, "/"), params)
{
}

lyric_build::TaskKey::TaskKey(
    const std::string &domain,
    const std::filesystem::path &path,
    const tempo_config::ConfigMap &params)
    : TaskKey(domain, path.string(), params)
{
}

lyric_build::TaskKey::TaskKey(
    const TaskId &taskId,
    const tempo_config::ConfigMap &params)
    : TaskKey(taskId.getDomain(), taskId.getId(), params)
{
}

lyric_build::TaskKey::TaskKey(const TaskKey &other)
    : m_priv(other.m_priv)
{
}

bool
lyric_build::TaskKey::isValid() const
{
    return m_priv != nullptr;
}

std::string
lyric_build::TaskKey::getDomain() const
{
    return m_priv? m_priv->domain : "";
}

std::string
lyric_build::TaskKey::getId() const
{
    return m_priv? m_priv->id : "";
}

tempo_config::ConfigMap
lyric_build::TaskKey::getParams() const
{
    return m_priv? m_priv->params : tempo_config::ConfigMap{};
}

std::string
lyric_build::TaskKey::toString() const
{
    if (m_priv != nullptr)
        return absl::StrCat(m_priv->domain, ":", m_priv->id, ":", m_priv->params.toString());
    return {};
}

lyric_build::TaskId
lyric_build::TaskKey::toTaskId() const
{
    if (m_priv != nullptr)
        return TaskId(m_priv->domain, m_priv->id);
    return {};

}

int
lyric_build::TaskKey::compare(const TaskKey &other) const
{
    if (m_priv) {
        if (other.m_priv == nullptr)
            return 1;
        auto domaincmp = m_priv->domain.compare(other.m_priv->domain);
        if (domaincmp != 0)
            return domaincmp;
        auto idcmp = m_priv->id.compare(other.m_priv->id);
        if (idcmp != 0)
            return idcmp;
        return m_priv->params.compare(other.m_priv->params);
    }
    return other.m_priv? -1 : 0;
}

bool
lyric_build::TaskKey::operator==(const TaskKey &other) const
{
    return compare(other) == 0;
}

bool
lyric_build::TaskKey::operator!=(const TaskKey &other) const
{
    return compare(other) != 0;
}

bool
lyric_build::TaskKey::operator<(const TaskKey &other) const
{
    return compare(other) < 0;
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const TaskKey &taskKey)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "TaskKey(domain=" << taskKey.getDomain()
        << ", id=" << taskKey.getId()
        << ", params=" << taskKey.getParams().toString()
        <<  ")";
    return std::move(message);
}

lyric_build::TaskState::TaskState()
    : m_status(Status::INVALID)
{
}

lyric_build::TaskState::TaskState(Status status, const BuildGeneration &generation, const std::string &hash)
    : m_status(status), m_generation(generation), m_hash(hash)
{
}

lyric_build::TaskState::TaskState(const lyric_build::TaskState &other)
    : m_status(other.m_status), m_generation(other.m_generation), m_hash(other.m_hash)
{
}

lyric_build::TaskState::Status
lyric_build::TaskState::getStatus() const
{
    return m_status;
}

lyric_build::BuildGeneration
lyric_build::TaskState::getGeneration() const
{
    return m_generation;
}

std::string
lyric_build::TaskState::getHash() const
{
    return m_hash;
}

std::string
lyric_build::TaskState::toString() const
{
    char const *valueStatus = nullptr;
    switch (m_status) {
        case TaskState::Status::INVALID:
            valueStatus = "INVALID";
            break;
        case lyric_build::TaskState::Status::QUEUED:
            valueStatus = "QUEUED";
            break;
        case lyric_build::TaskState::Status::RUNNING:
            valueStatus = "RUNNING";
            break;
        case lyric_build::TaskState::Status::BLOCKED:
            valueStatus = "BLOCKED";
            break;
        case lyric_build::TaskState::Status::COMPLETED:
            valueStatus = "COMPLETED";
            break;
        case TaskState::Status::FAILED:
            valueStatus = "FAILED";
            break;
    }
    return absl::Substitute("TaskState(status=$0, generation=$1, hash=$2)",
        valueStatus, m_generation.toString(), absl::BytesToHexString(m_hash));
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const lyric_build::TaskState &state)
{
    std::forward<tempo_utils::LogMessage>(message) << state.toString();
    return std::move(message);
}

lyric_build::ArtifactId::ArtifactId()
    : m_generation(),
      m_hash(),
      m_location()
{
}

lyric_build::ArtifactId::ArtifactId(
    const BuildGeneration &generation,
    const std::string &hash,
    const tempo_utils::Url &url)
    : m_generation(generation),
      m_hash(hash),
      m_location(url)
{
}

lyric_build::ArtifactId::ArtifactId(
    const BuildGeneration &generation,
    const std::string &hash,
    const tempo_utils::UrlPath &path)
    : ArtifactId(generation, hash, tempo_utils::Url::fromRelative(path.toString()))
{
}

lyric_build::ArtifactId::ArtifactId(const ArtifactId &other)
    : m_generation(other.m_generation),
      m_hash(other.m_hash),
      m_location(other.m_location)
{
}

bool
lyric_build::ArtifactId::isValid() const
{
    return m_generation.isValid() && !m_hash.empty();
}

lyric_build::BuildGeneration
lyric_build::ArtifactId::getGeneration() const
{
    return m_generation;
}

std::string
lyric_build::ArtifactId::getHash() const
{
    return m_hash;
}

tempo_utils::Url
lyric_build::ArtifactId::getLocation() const
{
    return m_location;
}

std::string
lyric_build::ArtifactId::toString() const
{
    return absl::StrCat(
        m_generation.toString(),
        ":",
        absl::BytesToHexString(m_hash),
        ":",
        m_location.toString());
}

bool
lyric_build::ArtifactId::operator==(const ArtifactId &other) const
{
    return m_generation == other.m_generation
        && m_hash == other.m_hash
        && m_location == other.m_location;
}

bool
lyric_build::ArtifactId::operator!=(const ArtifactId &other) const
{
    return !(*this == other);
}

bool
lyric_build::ArtifactId::operator<(const ArtifactId &other) const
{
    return toString() < other.toString();
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const ArtifactId &artifactId)
{
    std::forward<tempo_utils::LogMessage>(message) << artifactId.toString();
    return std::move(message);
}

lyric_build::TraceId::TraceId()
{
}

lyric_build::TraceId::TraceId(const std::string &hash, const std::string &domain, const std::string &id)
    : m_hash(hash),
      m_domain(domain),
      m_id(id)
{
}

lyric_build::TraceId::TraceId(const TraceId &other)
    : m_hash(other.m_hash),
      m_domain(other.m_domain),
      m_id(other.m_id)
{
}

bool
lyric_build::TraceId::isValid() const
{
    return !m_hash.empty() && !m_domain.empty();
}

std::string
lyric_build::TraceId::getHash() const
{
    return m_hash;
}

std::string
lyric_build::TraceId::getDomain() const
{
    return m_domain;
}

std::string
lyric_build::TraceId::getId() const
{
    return m_id;
}

std::string
lyric_build::TraceId::toString() const
{
    return absl::StrCat(
        absl::BytesToHexString(m_hash),
        ":",
        m_domain,
        ":",
        m_id);
}

bool
lyric_build::TraceId::operator==(const TraceId &other) const
{
    return m_hash == other.m_hash
        && m_domain == other.m_domain
        && m_id == other.m_id;
}

bool
lyric_build::TraceId::operator!=(const TraceId &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const TraceId &traceId)
{
    std::forward<tempo_utils::LogMessage>(message) << traceId.toString();
    return std::move(message);
}

lyric_build::AttrId::AttrId()
    : m_address(),
      m_type(METADATA_INVALID_OFFSET_U32)
{
}

lyric_build::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != METADATA_INVALID_OFFSET_U32);
}

lyric_build::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

lyric_build::NamespaceAddress
lyric_build::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
lyric_build::AttrId::getType() const
{
    return m_type;
}

bool
lyric_build::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}
