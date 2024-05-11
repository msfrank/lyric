
#include <absl/strings/str_join.h>

#include <lyric_parser/assignable.h>
#include <tempo_utils/log_stream.h>

lyric_parser::Assignable::Assignable()
  : m_type(AssignableType::INVALID),
    m_symbolUrl(),
    m_parameters()
{
}

lyric_parser::Assignable::Assignable(
    lyric_parser::AssignableType type,
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<Assignable> &parameters)
    : m_type(type),
      m_symbolUrl(symbolUrl),
      m_parameters(parameters)
{
}

lyric_parser::Assignable
lyric_parser::Assignable::forSingular(
    const lyric_common::SymbolPath &symbolPath,
    const std::vector<Assignable> &typeParameters)
{
    if (!symbolPath.isValid())
        return {};
    return Assignable(AssignableType::SINGULAR, lyric_common::SymbolUrl(symbolPath), typeParameters);
}

lyric_parser::Assignable
lyric_parser::Assignable::forSingular(
    std::initializer_list<std::string> symbolPath,
    const std::vector<Assignable> &typeParameters)
{
    return forSingular(lyric_common::SymbolPath(symbolPath), typeParameters);
}

lyric_parser::Assignable
lyric_parser::Assignable::forSingular(
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<Assignable> &typeParameters)
{
    if (!symbolUrl.isValid())
        return {};
    return Assignable(AssignableType::SINGULAR, symbolUrl, typeParameters);
}

lyric_parser::Assignable
lyric_parser::Assignable::forSingular(
    const tempo_utils::Url &location,
    const lyric_common::SymbolPath &path,
    const std::vector<Assignable> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::AssemblyLocation::fromUrl(location),
        lyric_common::SymbolPath(path)),
        parameters);
}

lyric_parser::Assignable
lyric_parser::Assignable::forSingular(
    const tempo_utils::Url &location,
    std::initializer_list<std::string> symbolPath,
    const std::vector<Assignable> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::AssemblyLocation::fromUrl(location),
        lyric_common::SymbolPath(symbolPath)),
        parameters);
}

lyric_parser::Assignable
lyric_parser::Assignable::forIntersection(const std::vector<Assignable> &members)
{
    TU_ASSERT (!members.empty());
    return Assignable(AssignableType::INTERSECTION, {}, members);
}

lyric_parser::Assignable
lyric_parser::Assignable::forUnion(const std::vector<Assignable> &members)
{
    TU_ASSERT (!members.empty());
    return Assignable(AssignableType::UNION, {}, members);
}

lyric_parser::Assignable
lyric_parser::Assignable::fromTypeDef(const lyric_common::TypeDef &typeDef)
{
    switch (typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            std::vector<lyric_parser::Assignable> parameters;
            for (auto iterator = typeDef.concreteArgumentsBegin(); iterator != typeDef.concreteArgumentsEnd(); iterator++) {
                parameters.push_back(Assignable::fromTypeDef(*iterator));
            }
            return lyric_parser::Assignable::forSingular(typeDef.getConcreteUrl(), parameters);
        }
        case lyric_common::TypeDefType::Placeholder: {
            auto templateUrl = typeDef.getPlaceholderTemplateUrl();
            auto location = templateUrl.getAssemblyLocation();
            auto path = templateUrl.getSymbolPath().getPath();
            auto placeholder = absl::StrCat(typeDef.getPlaceholderIndex());
            lyric_common::SymbolUrl placeholderUrl(location, lyric_common::SymbolPath(path, placeholder));
            std::vector<lyric_parser::Assignable> parameters;
            for (auto iterator = typeDef.placeholderArgumentsBegin(); iterator != typeDef.placeholderArgumentsEnd(); iterator++) {
                parameters.push_back(Assignable::fromTypeDef(*iterator));
            }
            return lyric_parser::Assignable::forSingular(placeholderUrl, parameters);
        }
        case lyric_common::TypeDefType::Intersection: {
            std::vector<lyric_parser::Assignable> members;
            for (auto iterator = typeDef.intersectionMembersBegin(); iterator != typeDef.intersectionMembersEnd(); iterator++) {
                members.push_back(Assignable::fromTypeDef(*iterator));
            }
            return lyric_parser::Assignable::forIntersection(members);
        }
        case lyric_common::TypeDefType::Union: {
            std::vector<lyric_parser::Assignable> members;
            for (auto iterator = typeDef.unionMembersBegin(); iterator != typeDef.unionMembersEnd(); iterator++) {
                members.push_back(Assignable::fromTypeDef(*iterator));
            }
            return lyric_parser::Assignable::forUnion(members);
        }
        default:
            return {};
    }
}

lyric_parser::Assignable::Assignable(const Assignable &other)
{
    m_type = other.m_type;
    m_symbolUrl = other.m_symbolUrl;
    m_parameters = other.m_parameters;
}

bool
lyric_parser::Assignable::isValid() const
{
    return m_type != AssignableType::INVALID;
}

lyric_parser::AssignableType
lyric_parser::Assignable::getType() const
{
    return m_type;
}

lyric_common::AssemblyLocation
lyric_parser::Assignable::getTypeLocation() const
{
    return m_symbolUrl.getAssemblyLocation();
}

lyric_common::SymbolPath
lyric_parser::Assignable::getTypePath() const
{
    if (m_type != AssignableType::SINGULAR)
        return {};
    return m_symbolUrl.getSymbolPath();
}

std::vector<lyric_parser::Assignable>
lyric_parser::Assignable::getTypeParameters() const
{
    if (m_type != AssignableType::SINGULAR)
        return {};
    return m_parameters;
}

std::vector<lyric_parser::Assignable>
lyric_parser::Assignable::getIntersection() const
{
    if (m_type != AssignableType::INTERSECTION)
        return {};
    return m_parameters;
}

std::vector<lyric_parser::Assignable>
lyric_parser::Assignable::getUnion() const
{
    if (m_type != AssignableType::UNION)
        return {};
    return m_parameters;
}

std::string
lyric_parser::Assignable::toString() const
{
    switch (m_type) {
        case AssignableType::SINGULAR: {
            if (!m_symbolUrl.isValid())
                return "???";
            std::string string = m_symbolUrl.toString();
            if (m_parameters.empty())
                return string;
            auto typeParamIterator = m_parameters.cbegin();
            string.append("[");
            string.append(typeParamIterator->toString());
            for (++typeParamIterator; typeParamIterator != m_parameters.cend(); typeParamIterator++) {
                string.append(", ");
                string.append(typeParamIterator->toString());
            }
            string.append("]");
            return string;
        }
        case AssignableType::INTERSECTION: {
            auto iterator = m_parameters.cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_parameters.cend(); iterator++) {
                string.append(" & ");
                string.append(iterator->toString());
            }
            return string;
        }
        case AssignableType::UNION: {
            auto iterator = m_parameters.cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_parameters.cend(); iterator++) {
                string.append(" | ");
                string.append(iterator->toString());
            }
            return string;
        }
        case AssignableType::INVALID:
            break;
    }
    return "???";
}

bool
lyric_parser::Assignable::operator==(const Assignable &other) const
{
    return m_type == other.m_type
        && m_symbolUrl == other.m_symbolUrl
        && m_parameters == other.m_parameters;
}

bool
lyric_parser::Assignable::operator!=(const Assignable &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const lyric_parser::Assignable &assignable)
{
    std::forward<tempo_utils::LogMessage>(message) << "Assignable(" << assignable.toString() << ")";
    return std::move(message);
}