
#include <tempo_utils/log_stream.h>

#include <lyric_serde/operation_path.h>

lyric_serde::OperationPathPart::OperationPathPart()
    : m_type(Type::INVALID)
{
}

lyric_serde::OperationPathPart::OperationPathPart(int index)
    : m_type(Type::INDEX),
      m_index(index)
{
}

lyric_serde::OperationPathPart::OperationPathPart(const std::pair<int,tu_uint32> &field)
    : m_type(Type::FIELD),
      m_field(field)
{
}

lyric_serde::OperationPathPart::OperationPathPart(std::string_view &id)
    : m_type(Type::ID),
      m_id(id)
{
}

lyric_serde::OperationPathPart::OperationPathPart(const OperationPathPart &other)
    : m_type(other.m_type),
      m_index(other.m_index),
      m_field(other.m_field),
      m_id(other.m_id)
{
}

bool
lyric_serde::OperationPathPart::isValid() const
{
    return m_type != Type::INVALID;
}

lyric_serde::OperationPathPart::Type
lyric_serde::OperationPathPart::getType() const
{
    return m_type;
}

bool
lyric_serde::OperationPathPart::isIndex() const
{
    return m_type == Type::INDEX;
}

bool
lyric_serde::OperationPathPart::isField() const
{
    return m_type == Type::FIELD;
}

bool
lyric_serde::OperationPathPart::isId() const
{
    return m_type == Type::ID;
}

int
lyric_serde::OperationPathPart::getIndex() const
{
    if (m_type == Type::INDEX)
        return m_index;
    return 0;
}

std::pair<int,tu_uint32>
lyric_serde::OperationPathPart::getField() const
{
    if (m_type == Type::FIELD)
        return m_field;
    return {-1,-1};
}

std::string
lyric_serde::OperationPathPart::getId() const
{
    if (m_type == Type::ID)
        return m_id;
    return {};
}

std::string_view
lyric_serde::OperationPathPart::idView() const
{
    if (m_type == Type::ID)
        return m_id;
    return {};
}

std::string
lyric_serde::OperationPathPart::toString() const
{
    switch (m_type) {
        case Type::INDEX:
            return absl::StrCat(m_index);
        case Type::FIELD:
            return absl::StrCat("[", m_field.first, "]", m_field.second);
        case Type::ID:
            return m_id;
        case Type::INVALID:
            return {};
    }
    TU_UNREACHABLE();
}

bool
lyric_serde::OperationPathPart::operator==(const OperationPathPart &other) const
{
    if (m_type != other.m_type)
        return false;
    switch (m_type) {
        case Type::INVALID:
            return true;
        case Type::INDEX:
            return m_index == other.m_index;
        case Type::FIELD:
            return m_field == other.m_field;
        case Type::ID:
            return m_id == other.m_id;
        default:
            return false;
    }
}

bool
lyric_serde::OperationPathPart::operator!=(const OperationPathPart &other) const
{
    return !(*this == other);
}

lyric_serde::OperationPathPart
lyric_serde::OperationPathPart::fromString(std::string_view s)
{
    if (s.empty())
        return {};
    auto head = s.front();
    if (head == '[') {
        auto nsEnd = s.find(']', 1);
        if (nsEnd == std::string_view::npos)
            return {};
        std::pair<int,tu_uint32> field;
        if (!absl::SimpleAtoi(s.substr(1, nsEnd - 1), &field.first))
            return {};
        if (!absl::SimpleAtoi(s.substr(nsEnd + 1), &field.second))
            return {};
        return OperationPathPart(field);
    }
    else if (head == '-' || head == '+' || std::isdigit(head)) {
        int i;
        if (!absl::SimpleAtoi(s, &i))
            return {};
        return OperationPathPart(i);
    } else {
        return OperationPathPart(s);
    }
}

lyric_serde::OperationPath::OperationPath()
{
}

lyric_serde::OperationPath::OperationPath(const tempo_utils::UrlPath &path)
    : m_path(path)
{
}

lyric_serde::OperationPath::OperationPath(const lyric_serde::OperationPath &other)
    : m_path(other.m_path)
{
}

bool
lyric_serde::OperationPath::isValid() const
{
    return m_path.isValid();
}

bool
lyric_serde::OperationPath::isEmpty() const
{
    return m_path.isEmpty();
}

int
lyric_serde::OperationPath::numParts() const
{
    return m_path.numParts();
}

lyric_serde::OperationPathPart
lyric_serde::OperationPath::getPart(int index) const
{
    return OperationPathPart::fromString(m_path.getPart(index).decode());
}

std::string_view
lyric_serde::OperationPath::partView(int index) const
{
    return m_path.partView(index);
}

lyric_serde::OperationPath
lyric_serde::OperationPath::getInit() const
{
    return OperationPath(m_path.getInit());
}

lyric_serde::OperationPath
lyric_serde::OperationPath::getTail() const
{
    return OperationPath(m_path.getTail());
}

lyric_serde::OperationPathPart
lyric_serde::OperationPath::getLast() const
{
    return OperationPathPart::fromString(m_path.getLast().decode());
}

std::string_view
lyric_serde::OperationPath::lastView() const
{
    return m_path.lastView();
}

std::string_view
lyric_serde::OperationPath::pathView() const
{
    return m_path.pathView();
}

lyric_serde::OperationPath
lyric_serde::OperationPath::traverse(const OperationPathPart &part)
{
    auto path = m_path.traverse(tempo_utils::UrlPathPart(part.toString()));
    return OperationPath(path);
}

std::string
lyric_serde::OperationPath::toString() const
{
    return m_path.toString();
}

tempo_utils::Url
lyric_serde::OperationPath::toUrl() const
{
    return tempo_utils::Url::fromRelative(m_path.pathView());
}

bool
lyric_serde::OperationPath::operator==(const lyric_serde::OperationPath &other) const
{
    return m_path == other.m_path;
}

bool
lyric_serde::OperationPath::operator!=(const lyric_serde::OperationPath &other) const
{
    return !(*this == other);
}

lyric_serde::OperationPath
lyric_serde::OperationPath::fromString(std::string_view s)
{
    auto path = tempo_utils::UrlPath::fromString(s);
    if (!path.isValid())
        return {};
    for (int i = 0; i < path.numParts(); i++) {
        auto part = OperationPathPart::fromString(path.partView(i));
        if (!part.isValid())
            return {};
    }
    return OperationPath(path);
}

tempo_utils::LogMessage&&
lyric_serde::operator<<(tempo_utils::LogMessage &&message, const lyric_serde::OperationPath &operationPath)
{
    std::forward<tempo_utils::LogMessage>(message) << operationPath.toString();
    return std::move(message);
}
