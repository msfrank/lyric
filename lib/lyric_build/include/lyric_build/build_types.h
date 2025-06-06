#ifndef LYRIC_BUILD_BUILD_TYPES_H
#define LYRIC_BUILD_BUILD_TYPES_H

#include <chrono>
#include <filesystem>

#include <lyric_common/module_location.h>
#include <tempo_config/config_types.h>
#include <tempo_tracing/tempo_spanset.h>
#include <tempo_utils/uuid.h>

namespace lyric_build {

    constexpr tu_uint32 METADATA_INVALID_OFFSET_U32     = 0xffffffff;
    constexpr tu_uint16 METADATA_INVALID_OFFSET_U16     = 0xffff;
    constexpr tu_uint8 METADATA_INVALID_OFFSET_U8       = 0xff;

    enum class MetadataVersion {
        Unknown,
        Version1,
    };

    enum class EntryType {
        Unknown,
        File,
        Directory,
        Link,
        LinkOverride,
    };

    /**
     * BuildGeneration contains the globally-unique id which identifies the build invocation
     * and a timestamp indicating when the build invocation occurred.
     */
    class BuildGeneration {

    public:
        BuildGeneration();
        explicit BuildGeneration(const tempo_utils::UUID &uuid);
        BuildGeneration(const BuildGeneration &other);

        bool isValid() const;
        tempo_utils::UUID getUuid() const;

        static BuildGeneration create();

    private:
        tempo_utils::UUID m_uuid;
    };

    /**
     * TaskKey is used internally to uniquely identify a task which is to be executed as part of a build.
     */
    class TaskKey {

    public:
        TaskKey();
        TaskKey(const std::string &domain, const std::string &id, const tempo_config::ConfigMap &params = {});
        TaskKey(const std::string &domain, const std::vector<std::string> &parts, const tempo_config::ConfigMap &params = {});
        TaskKey(const std::string &domain, const std::filesystem::path &path, const tempo_config::ConfigMap &params = {});
        TaskKey(const TaskKey &other);

        bool isValid() const;
        std::string getDomain() const;
        std::string getId() const;
        tempo_config::ConfigMap getParams() const;

        std::string toString() const;

        bool operator==(const TaskKey &other) const;
        bool operator!=(const TaskKey &other) const;
        bool operator<(const TaskKey &other) const;

        int compare(const TaskKey &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TaskKey &taskKey) {
            return H::combine(std::move(h), taskKey.m_priv->domain, taskKey.m_priv->id, taskKey.m_priv->params);
        }

    private:
        struct Priv {
            std::string domain;
            std::string id;
            tempo_config::ConfigMap params;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * TaskId describes a task executed by the build system.
     */
    class TaskId {

    public:
        TaskId();
        TaskId(const std::string &domain, const std::string &id = {});
        TaskId(const std::string &domain, const std::vector<std::string> &parts);
        TaskId(const std::string &domain, const std::filesystem::path &path);
        TaskId(const TaskId &other);

        bool isValid() const;
        std::string getDomain() const;
        std::string getId() const;

        std::string toString() const;

        bool operator==(const TaskId &other) const;
        bool operator!=(const TaskId &other) const;
        bool operator<(const TaskId &other) const;

        static TaskId fromString(const std::string &s);

        int compare(const TaskId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TaskId &taskId) {
            return H::combine(std::move(h), taskId.m_priv->domain, taskId.m_priv->id);
        }

    private:
        struct Priv {
            std::string domain;
            std::string id;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * TaskState contains the execution state for a task.
     */
    class TaskState {

    public:
        enum class Status {
            INVALID,
            QUEUED,
            RUNNING,
            BLOCKED,
            COMPLETED,
            FAILED,
        };

        TaskState();
        TaskState(Status status, const tempo_utils::UUID &generation, const std::string &hash);
        TaskState(const TaskState &other);

        Status getStatus() const;
        tempo_utils::UUID getGeneration() const;
        std::string getHash() const;

        std::string toString() const;

    private:
        Status m_status;
        tempo_utils::UUID m_generation;
        std::string m_hash;
    };

    /**
     * ArtifactId uniquely identifies an artifact in the build cache.
     */
    class ArtifactId {

    public:
        ArtifactId();
        ArtifactId(const tempo_utils::UUID &generation, const std::string &hash, const tempo_utils::Url &url);
        ArtifactId(const tempo_utils::UUID &generation, const std::string &hash, const tempo_utils::UrlPath &path);
        ArtifactId(const ArtifactId &other);

        bool isValid() const;
        tempo_utils::UUID getGeneration() const;
        std::string getHash() const;
        tempo_utils::Url getLocation() const;

        std::string toString() const;

        bool operator==(const ArtifactId &other) const;
        bool operator!=(const ArtifactId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const ArtifactId &artifactId) {
            return H::combine(std::move(h), artifactId.m_generation, artifactId.m_hash, artifactId.m_location);
        }

    private:
        tempo_utils::UUID m_generation;
        std::string m_hash;
        tempo_utils::Url m_location;
    };

    /**
     * TraceId identifies the generation containing the build artifacts for the specified task id
     * (the domain and id) with the specified task hash.
     */
    class TraceId {

    public:
        TraceId();
        TraceId(const std::string &hash, const std::string &domain, const std::string &id);
        TraceId(const TraceId &other);

        bool isValid() const;
        std::string getHash() const;
        std::string getDomain() const;
        std::string getId() const;

        std::string toString() const;

        bool operator==(const TraceId &other) const;
        bool operator!=(const TraceId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TraceId &traceId) {
            return H::combine(std::move(h), traceId.m_hash, traceId.m_domain, traceId.m_id);
        }

    private:
        std::string m_hash;
        std::string m_domain;
        std::string m_id;
    };

    bool operator<(const ArtifactId &lhs, const ArtifactId &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskId &taskId);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskKey &taskKey);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskState &state);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const ArtifactId &artifactId);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TraceId &traceId);

    struct NamespaceAddress {
    public:
        NamespaceAddress() : u32(METADATA_INVALID_OFFSET_U32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != METADATA_INVALID_OFFSET_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = METADATA_INVALID_OFFSET_U32;
    };

    struct AttrAddress {
    public:
        AttrAddress() : u32(METADATA_INVALID_OFFSET_U32) {};
        explicit AttrAddress(tu_uint32 u32) : u32(u32) {};
        AttrAddress(const AttrAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != METADATA_INVALID_OFFSET_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const AttrAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = METADATA_INVALID_OFFSET_U32;
    };

    class AttrId {
    public:
        AttrId();
        AttrId(const NamespaceAddress &address, tu_uint32 type);
        AttrId(const AttrId &other);

        NamespaceAddress getAddress() const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            return H::combine(std::move(h), id.m_address.getAddress(), id.m_type);
        }

    private:
        NamespaceAddress m_address;
        tu_uint32 m_type;
    };

    // forward declarations
    namespace internal {
        class MetadataReader;
    }
}

#endif // LYRIC_BUILD_BUILD_TYPES_H