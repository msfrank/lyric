#ifndef LYRIC_SERDE_OPERATION_PATH_H
#define LYRIC_SERDE_OPERATION_PATH_H

#include <string>

#include <tempo_utils/log_message.h>
#include <tempo_utils/url.h>

namespace lyric_serde {

    class OperationPathPart {

    public:
        OperationPathPart();
        explicit OperationPathPart(int index);
        explicit OperationPathPart(const std::pair<int,tu_uint32> &field);
        explicit OperationPathPart(std::string_view &id);
        OperationPathPart(const OperationPathPart &other);

        bool isValid() const;

        enum class Type {
            INVALID,
            INDEX,
            FIELD,
            ID,
        };

        Type getType() const;
        bool isIndex() const;
        bool isField() const;
        bool isId() const;
        int getIndex() const;
        std::pair<int,tu_uint32> getField() const;
        std::string getId() const;
        std::string_view idView() const;

        std::string toString() const;

        bool operator==(const OperationPathPart &other) const;
        bool operator!=(const OperationPathPart &other) const;

        static OperationPathPart fromString(std::string_view s);

    private:
        Type m_type;
        int m_index;
        std::pair<int,tu_uint32> m_field;
        std::string m_id;
    };

    class OperationPath {
    public:
        OperationPath();
        OperationPath(const OperationPath &other);

        bool isValid() const;

        bool isEmpty() const;

        int numParts() const;
        OperationPathPart getPart(int index) const;
        std::string_view partView(int index) const;

        OperationPath getInit() const;
        OperationPath getTail() const;

        OperationPathPart getLast() const;
        std::string_view lastView() const;

        std::string_view pathView() const;

        OperationPath traverse(const OperationPathPart &part);

        std::string toString() const;
        tempo_utils::Url toUrl() const;

        bool operator==(const OperationPath &other) const;
        bool operator!=(const OperationPath &other) const;

        static OperationPath fromString(std::string_view s);

        template <typename H>
        friend H AbslHashValue(H h, const OperationPath &path) {
            return H::combine(std::move(h), path.m_path);
        }

    private:
        tempo_utils::UrlPath m_path;

        OperationPath(const tempo_utils::UrlPath &path);

    public:
        template<class... Args>
        OperationPath traverse(const OperationPathPart &part, Args... args)
        {
            auto path = traverse(part);
            return path.traverse(args...);
        }
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const OperationPath &operationPath);
}

#endif // LYRIC_SERDE_OPERATION_PATH_H