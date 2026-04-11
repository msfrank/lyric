#ifndef LYRIC_BUILD_BUILD_TYPES_H
#define LYRIC_BUILD_BUILD_TYPES_H

#include <array>

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
        Link,
        LinkOverride,
    };

    enum class TaskState {
        INVALID,
        QUEUED,
        RUNNING,
        BLOCKED,
        COMPLETED,
        FAILED,
    };

    /**
     * BuildGeneration contains the globally-unique id which identifies the build invocation
     * and a timestamp indicating when the build invocation occurred.
     */
    class BuildGeneration final {

    public:
        BuildGeneration() = default;
        explicit BuildGeneration(const tempo_utils::UUID &uuid);
        BuildGeneration(const BuildGeneration &other);

        bool isValid() const;
        std::vector<tu_uint8> toBytes() const;
        std::string toString() const;

        int compare(const BuildGeneration &other) const;
        bool operator==(const BuildGeneration &other) const;
        bool operator!=(const BuildGeneration &other) const;
        bool operator<(const BuildGeneration &other) const;

        static BuildGeneration create();
        static BuildGeneration parse(std::string_view s);

        template <typename H>
        friend H AbslHashValue(H h, const BuildGeneration &generation) {
            return H::combine(std::move(h), generation.m_uuid);
        }

    private:
        tempo_utils::UUID m_uuid;
    };

    class TaskHash final {
    public:
        TaskHash() = default;
        explicit TaskHash(std::vector<tu_uint8> taskHash);
        TaskHash(const TaskHash &other);

        bool isValid() const;
        std::span<tu_uint8> bytesView() const;
        std::vector<tu_uint8> toBytes() const;
        std::string toString() const;

        int compare(const TaskHash &other) const;
        bool operator==(const TaskHash &other) const;
        bool operator!=(const TaskHash &other) const;
        bool operator<(const TaskHash &other) const;

        static TaskHash parse(std::string_view s);

        template <typename H>
        friend H AbslHashValue(H h, const TaskHash &taskHash) {
            if (taskHash.m_hash)
                return H::combine(std::move(h), *taskHash.m_hash);
            return H::combine(std::move(h), taskHash.m_hash);
        }

    private:
        std::shared_ptr<std::vector<tu_uint8>> m_hash;
    };

    /**
     * TaskId describes a task executed by the build system.
     */
    class TaskId final {

    public:
        TaskId() = default;
        explicit TaskId(const std::string &domain, const std::string &id = {});
        TaskId(const std::string &domain, const std::vector<std::string> &parts);
        TaskId(const std::string &domain, const std::filesystem::path &path);
        TaskId(const std::string &domain, const tempo_utils::UrlPath &urlPath);
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
            if (taskId.m_priv)
                return H::combine(std::move(h), taskId.m_priv->domain, taskId.m_priv->id);
            return H::combine(std::move(h), taskId.m_priv);
        }

    private:
        struct Priv {
            std::string domain;
            std::string id;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * TaskKey is used internally to uniquely identify a task which is to be executed as part of a build.
     */
    class TaskKey final {

    public:
        TaskKey() = default;
        explicit TaskKey(const TaskId &taskId, const tempo_config::ConfigMap &params = {});
        TaskKey(const std::string &domain, const std::string &id, const tempo_config::ConfigMap &params = {});
        TaskKey(const std::string &domain, const std::vector<std::string> &parts, const tempo_config::ConfigMap &params = {});
        TaskKey(const std::string &domain, const std::filesystem::path &path, const tempo_config::ConfigMap &params = {});
        TaskKey(const TaskKey &other);

        bool isValid() const;
        std::string getDomain() const;
        std::string getId() const;
        tempo_config::ConfigMap getParams() const;

        std::string toString() const;
        TaskId toTaskId() const;

        int compare(const TaskKey &other) const;
        bool operator==(const TaskKey &other) const;
        bool operator!=(const TaskKey &other) const;
        bool operator<(const TaskKey &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TaskKey &taskKey) {
            if (taskKey.m_priv)
                return H::combine(std::move(h), taskKey.m_priv->domain, taskKey.m_priv->id, taskKey.m_priv->params);
            return H::combine(std::move(h), taskKey.m_priv);
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
     * TaskReference identifies a task within a specific build generation.
     */
    class TaskReference final {

    public:
        TaskReference() = default;
        TaskReference(const BuildGeneration &generation, const TaskKey &key);
        TaskReference(const TaskReference &other);

        bool isValid() const;
        BuildGeneration getGeneration() const;
        TaskKey getKey() const;

        std::string toString() const;

        int compare(const TaskReference &other) const;
        bool operator==(const TaskReference &other) const;
        bool operator!=(const TaskReference &other) const;
        bool operator<(const TaskReference &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TaskReference &ref) {
            if (ref.m_priv)
                return H::combine(std::move(h), ref.m_priv->generation, ref.m_priv->key);
            return H::combine(std::move(h), ref.m_priv);
        }

    private:
        struct Priv {
            BuildGeneration generation;
            TaskKey key;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * TaskData contains the execution state for a task.
     */
    class TaskData final {
    public:
        TaskData() = default;
        TaskData(TaskState state, const BuildGeneration &generation);
        TaskData(TaskState state, const BuildGeneration &generation, const TaskHash &hash);
        TaskData(const TaskData &other);

        TaskState getState() const;
        BuildGeneration getGeneration() const;
        TaskHash getHash() const;

        std::string toString() const;

    private:
        struct Priv {
            TaskState state;
            BuildGeneration generation;
            TaskHash hash;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * ArtifactId uniquely identifies an artifact in the build cache.
     */
    class ArtifactId final {

    public:
        ArtifactId() = default;
        ArtifactId(const BuildGeneration &generation, const TaskHash &hash, const tempo_utils::Url &url);
        ArtifactId(const BuildGeneration &generation, const TaskHash &hash, const tempo_utils::UrlPath &path);
        ArtifactId(const ArtifactId &other);

        bool isValid() const;
        BuildGeneration getGeneration() const;
        TaskHash getHash() const;
        tempo_utils::Url getLocation() const;

        std::string toString() const;

        int compare(const ArtifactId &other) const;
        bool operator==(const ArtifactId &other) const;
        bool operator!=(const ArtifactId &other) const;
        bool operator<(const ArtifactId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const ArtifactId &artifactId) {
            if (artifactId.m_priv)
                return H::combine(std::move(h), artifactId.m_priv->generation, artifactId.m_priv->hash, artifactId.m_priv->location);
            return H::combine(std::move(h), artifactId.m_priv);
        }

    private:
        struct Priv {
            BuildGeneration generation;
            TaskHash hash;
            tempo_utils::Url location;
        };
        std::shared_ptr<Priv> m_priv;
    };

    /**
     * TraceId identifies the generation containing the build artifacts for the specified task id
     * (the domain and id) with the specified task hash.
     */
    class TraceId final {

    public:
        TraceId() = default;
        TraceId(const TaskHash &hash, const TaskKey &key);
        TraceId(const TraceId &other);

        bool isValid() const;
        TaskHash getHash() const;
        TaskKey getKey() const;

        std::string toString() const;

        int compare(const TraceId &other) const;
        bool operator==(const TraceId &other) const;
        bool operator!=(const TraceId &other) const;
        bool operator<(const TraceId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const TraceId &traceId) {
            if (traceId.m_priv)
                return H::combine(std::move(h), traceId.m_priv->hash, traceId.m_priv->key);
            return H::combine(std::move(h), traceId.m_priv);
        }

    private:
        struct Priv {
            TaskHash hash;
            TaskKey key;
        };
        std::shared_ptr<Priv> m_priv;
    };


    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskId &taskId);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskKey &taskKey);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TaskData &state);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const ArtifactId &artifactId);
    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const TraceId &traceId);

    struct NamespaceAddress final {
        NamespaceAddress() : u32(METADATA_INVALID_OFFSET_U32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != METADATA_INVALID_OFFSET_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = METADATA_INVALID_OFFSET_U32;
    };

    struct AttrAddress final {
        AttrAddress() : u32(METADATA_INVALID_OFFSET_U32) {};
        explicit AttrAddress(tu_uint32 u32) : u32(u32) {};
        AttrAddress(const AttrAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != METADATA_INVALID_OFFSET_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const AttrAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = METADATA_INVALID_OFFSET_U32;
    };

    class AttrId final {
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