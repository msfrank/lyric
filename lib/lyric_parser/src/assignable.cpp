
#include <absl/strings/str_join.h>

#include <lyric_parser/assignable.h>
#include <tempo_utils/log_stream.h>

lyric_parser::TypeSpec::TypeSpec()
  : m_type(AssignableType::INVALID),
    m_symbolUrl(),
    m_parameters()
{
}

lyric_parser::TypeSpec::TypeSpec(
    lyric_parser::AssignableType type,
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<TypeSpec> &parameters)
    : m_type(type),
      m_symbolUrl(symbolUrl),
      m_parameters(parameters)
{
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forSingular(
    const lyric_common::SymbolPath &symbolPath,
    const std::vector<TypeSpec> &typeParameters)
{
    if (!symbolPath.isValid())
        return {};
    return TypeSpec(AssignableType::SINGULAR, lyric_common::SymbolUrl(symbolPath), typeParameters);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forSingular(
    std::initializer_list<std::string> symbolPath,
    const std::vector<TypeSpec> &typeParameters)
{
    return forSingular(lyric_common::SymbolPath(symbolPath), typeParameters);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forSingular(
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<TypeSpec> &typeParameters)
{
    if (!symbolUrl.isValid())
        return {};
    return TypeSpec(AssignableType::SINGULAR, symbolUrl, typeParameters);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forSingular(
    const tempo_utils::Url &location,
    const lyric_common::SymbolPath &path,
    const std::vector<TypeSpec> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::AssemblyLocation::fromUrl(location),
        lyric_common::SymbolPath(path)),
        parameters);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forSingular(
    const tempo_utils::Url &location,
    std::initializer_list<std::string> symbolPath,
    const std::vector<TypeSpec> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::AssemblyLocation::fromUrl(location),
        lyric_common::SymbolPath(symbolPath)),
        parameters);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forIntersection(const std::vector<TypeSpec> &members)
{
    TU_ASSERT (!members.empty());
    return TypeSpec(AssignableType::INTERSECTION, {}, members);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::forUnion(const std::vector<TypeSpec> &members)
{
    TU_ASSERT (!members.empty());
    return TypeSpec(AssignableType::UNION, {}, members);
}

lyric_parser::TypeSpec
lyric_parser::TypeSpec::fromTypeDef(const lyric_common::TypeDef &typeDef)
{
    switch (typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            std::vector<lyric_parser::TypeSpec> parameters;
            for (auto iterator = typeDef.concreteArgumentsBegin(); iterator != typeDef.concreteArgumentsEnd(); iterator++) {
                parameters.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_parser::TypeSpec::forSingular(typeDef.getConcreteUrl(), parameters);
        }
        case lyric_common::TypeDefType::Placeholder: {
            auto templateUrl = typeDef.getPlaceholderTemplateUrl();
            auto location = templateUrl.getAssemblyLocation();
            auto path = templateUrl.getSymbolPath().getPath();
            auto placeholder = absl::StrCat(typeDef.getPlaceholderIndex());
            lyric_common::SymbolUrl placeholderUrl(location, lyric_common::SymbolPath(path, placeholder));
            std::vector<lyric_parser::TypeSpec> parameters;
            for (auto iterator = typeDef.placeholderArgumentsBegin(); iterator != typeDef.placeholderArgumentsEnd(); iterator++) {
                parameters.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_parser::TypeSpec::forSingular(placeholderUrl, parameters);
        }
        case lyric_common::TypeDefType::Intersection: {
            std::vector<lyric_parser::TypeSpec> members;
            for (auto iterator = typeDef.intersectionMembersBegin(); iterator != typeDef.intersectionMembersEnd(); iterator++) {
                members.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_parser::TypeSpec::forIntersection(members);
        }
        case lyric_common::TypeDefType::Union: {
            std::vector<lyric_parser::TypeSpec> members;
            for (auto iterator = typeDef.unionMembersBegin(); iterator != typeDef.unionMembersEnd(); iterator++) {
                members.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_parser::TypeSpec::forUnion(members);
        }
        default:
            return {};
    }
}

lyric_parser::TypeSpec::TypeSpec(const TypeSpec &other)
{
    m_type = other.m_type;
    m_symbolUrl = other.m_symbolUrl;
    m_parameters = other.m_parameters;
}

bool
lyric_parser::TypeSpec::isValid() const
{
    return m_type != AssignableType::INVALID;
}

lyric_parser::AssignableType
lyric_parser::TypeSpec::getType() const
{
    return m_type;
}

lyric_common::AssemblyLocation
lyric_parser::TypeSpec::getTypeLocation() const
{
    return m_symbolUrl.getAssemblyLocation();
}

lyric_common::SymbolPath
lyric_parser::TypeSpec::getTypePath() const
{
    if (m_type != AssignableType::SINGULAR)
        return {};
    return m_symbolUrl.getSymbolPath();
}

std::vector<lyric_parser::TypeSpec>
lyric_parser::TypeSpec::getTypeParameters() const
{
    if (m_type != AssignableType::SINGULAR)
        return {};
    return m_parameters;
}

std::vector<lyric_parser::TypeSpec>
lyric_parser::TypeSpec::getIntersection() const
{
    if (m_type != AssignableType::INTERSECTION)
        return {};
    return m_parameters;
}

std::vector<lyric_parser::TypeSpec>
lyric_parser::TypeSpec::getUnion() const
{
    if (m_type != AssignableType::UNION)
        return {};
    return m_parameters;
}

std::string
lyric_parser::TypeSpec::toString() const
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
lyric_parser::TypeSpec::operator==(const TypeSpec &other) const
{
    return m_type == other.m_type
        && m_symbolUrl == other.m_symbolUrl
        && m_parameters == other.m_parameters;
}

bool
lyric_parser::TypeSpec::operator!=(const TypeSpec &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const lyric_parser::TypeSpec &assignable)
{
    std::forward<tempo_utils::LogMessage>(message) << "TypeSpec(" << assignable.toString() << ")";
    return std::move(message);
}