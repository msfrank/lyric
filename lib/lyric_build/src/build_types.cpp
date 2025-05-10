
#include <absl/strings/escaping.h>
#include <absl/strings/str_join.h>
#include <absl/strings/substitute.h>

#include <lyric_build/build_types.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_stream.h>

lyric_build::BuildGeneration::BuildGeneration()
{
}

lyric_build::BuildGeneration::BuildGeneration(const tempo_utils::UUID &uuid, tu_int64 createTime)
    : m_uuid(uuid),
      m_createTime(createTime)
{
    TU_ASSERT (m_uuid.isValid());
    TU_ASSERT (m_createTime > 0);
}

lyric_build::BuildGeneration::BuildGeneration(const lyric_build::BuildGeneration &other)
    : m_uuid(other.m_uuid),
      m_createTime(other.m_createTime)
{
}

bool
lyric_build::BuildGeneration::isValid() const
{
    return m_uuid.isValid();
}

tempo_utils::UUID
lyric_build::BuildGeneration::getUuid() const
{
    return m_uuid;
}

tu_int64
lyric_build::BuildGeneration::getCreateTime() const
{
    return m_createTime;
}

lyric_build::BuildGeneration
lyric_build::BuildGeneration::create()
{
    auto uuid = tempo_utils::UUID::randomUUID();
    auto createTime = tempo_utils::millis_since_epoch();
    return BuildGeneration(uuid, createTime);
}

lyric_build::TaskKey::TaskKey()
    : m_domain(), m_id()
{
}

lyric_build::TaskKey::TaskKey(
    const std::string &domain,
    const std::string &id,
    const tempo_config::ConfigMap &params)
    : m_domain(domain),
      m_id(id),
      m_params(params)
{
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

lyric_build::TaskKey::TaskKey(const lyric_build::TaskKey &other)
    : m_domain(other.m_domain),
      m_id(other.m_id),
      m_params(other.m_params)
{
}

bool
lyric_build::TaskKey::isValid() const
{
    return !m_domain.empty();
}

std::string
lyric_build::TaskKey::getDomain() const
{
    return m_domain;
}

std::string
lyric_build::TaskKey::getId() const
{
    return m_id;
}

tempo_config::ConfigMap
lyric_build::TaskKey::getParams() const
{
    return m_params;
}

std::string
lyric_build::TaskKey::toString() const
{
    return absl::StrCat(m_domain, ":", m_id, ":", m_params.toString());
}

bool
lyric_build::TaskKey::operator==(const lyric_build::TaskKey &other) const
{
    return m_domain == other.m_domain
        && m_id == other.m_id
        && m_params == other.m_params;
}

bool
lyric_build::TaskKey::operator!=(const lyric_build::TaskKey &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const lyric_build::TaskKey &taskKey)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "TaskKey(domain=" << taskKey.getDomain()
        << ", id=" << taskKey.getId()
        << ", params=" << taskKey.getParams().toString()
        <<  ")";
    return std::move(message);
}

bool
lyric_build::operator<(const lyric_build::TaskKey &lhs, const lyric_build::TaskKey &rhs)
{
    return lhs.toString() < rhs.toString();
}

lyric_build::TaskId::TaskId()
    : m_domain(), m_id()
{
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::string &id)
    : m_domain(domain), m_id(id)
{
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::vector<std::string> &parts)
    : lyric_build::TaskId(domain, absl::StrJoin(parts, "/"))
{
}

lyric_build::TaskId::TaskId(const std::string &domain, const std::filesystem::path &path)
    : lyric_build::TaskId(domain, path.string())
{
}

lyric_build::TaskId::TaskId(const lyric_build::TaskId &other)
    : m_domain(other.m_domain),
      m_id(other.m_id)
{
}

bool
lyric_build::TaskId::isValid() const
{
    return !m_domain.empty();
}

std::string
lyric_build::TaskId::getDomain() const
{
    return m_domain;
}

std::string
lyric_build::TaskId::getId() const
{
    return m_id;
}

std::string
lyric_build::TaskId::toString() const
{
    return absl::StrCat(m_domain, ":", m_id);
}

bool
lyric_build::TaskId::operator==(const lyric_build::TaskId &other) const
{
    return m_domain == other.m_domain && m_id == other.m_id;
}

bool
lyric_build::TaskId::operator!=(const lyric_build::TaskId &other) const
{
    return !(*this == other);
}

lyric_build::TaskId
lyric_build::TaskId::fromString(const std::string &s)
{
    auto split = s.find_first_of(':');
    if (split == std::string::npos)
        return lyric_build::TaskId(s, std::string(""));
    auto domain = s.substr(0, split);
    auto id = s.substr(split + 1);
    return lyric_build::TaskId(domain, id);
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const lyric_build::TaskId &taskId)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "TaskId(domain=" << taskId.getDomain()
        << ", id=" << taskId.getId()
        <<  ")";
    return std::move(message);
}

bool
lyric_build::operator<(const lyric_build::TaskId &lhs, const lyric_build::TaskId &rhs)
{
    return lhs.toString() < rhs.toString();
}

lyric_build::TaskState::TaskState()
    : m_status(lyric_build::TaskState::Status::INVALID)
{
}

lyric_build::TaskState::TaskState(Status status, const tempo_utils::UUID &generation, const std::string &hash)
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

tempo_utils::UUID
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
    const tempo_utils::UUID &generation,
    const std::string &hash,
    const tempo_utils::Url &location)
    : m_generation(generation),
      m_hash(hash),
      m_location(location)
{
}

// lyric_build::ArtifactId::ArtifactId(
//     const boost::uuids::uuid &generation,
//     const std::string &hash,
//     const std::vector<std::string> &path)
//     : m_generation(generation),
//       m_hash(hash)
// {
//     std::string p("/");
//     absl::StrAppend(&p, absl::StrJoin(path, "/"));
//     m_location = tempo_utils::Url::fromRelative(p);
// }
//
// lyric_build::ArtifactId::ArtifactId(
//     const boost::uuids::uuid &generation,
//     const std::string &hash,
//     const std::string &path)
//     : ArtifactId(generation, hash, absl::StrSplit(path, "/"))
// {
// }
//
// lyric_build::ArtifactId::ArtifactId(
//     const boost::uuids::uuid &generation,
//     const std::string &hash,
//     const std::filesystem::path &path)
//     : ArtifactId(generation, hash, path.string())
// {
// }

lyric_build::ArtifactId::ArtifactId(const lyric_build::ArtifactId &other)
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

tempo_utils::UUID
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
lyric_build::ArtifactId::operator==(const lyric_build::ArtifactId &other) const
{
    return m_generation == other.m_generation
        && m_hash == other.m_hash
        && m_location == other.m_location;
}

bool
lyric_build::ArtifactId::operator!=(const lyric_build::ArtifactId &other) const
{
    return !(*this == other);
}

bool
lyric_build::operator<(const lyric_build::ArtifactId &lhs, const lyric_build::ArtifactId &rhs)
{
    return lhs.toString() < rhs.toString();
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const lyric_build::ArtifactId &artifactId)
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

lyric_build::TraceId::TraceId(const lyric_build::TraceId &other)
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
lyric_build::TraceId::operator==(const lyric_build::TraceId &other) const
{
    return m_hash == other.m_hash
        && m_domain == other.m_domain
        && m_id == other.m_id;
}

bool
lyric_build::TraceId::operator!=(const lyric_build::TraceId &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&&
lyric_build::operator<<(tempo_utils::LogMessage &&message, const lyric_build::TraceId &traceId)
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
