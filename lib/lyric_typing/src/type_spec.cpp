
#include <absl/strings/str_join.h>

#include <lyric_typing/type_spec.h>
#include <tempo_utils/log_stream.h>

lyric_typing::TypeSpec::TypeSpec()
  : m_type(TypeSpecType::Invalid),
    m_symbolUrl(),
    m_parameters()
{
}

lyric_typing::TypeSpec::TypeSpec(
    lyric_typing::TypeSpecType type,
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<TypeSpec> &parameters)
    : m_type(type),
      m_symbolUrl(symbolUrl),
      m_parameters(parameters)
{
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forSingular(
    const lyric_common::SymbolPath &symbolPath,
    const std::vector<TypeSpec> &typeParameters)
{
    if (!symbolPath.isValid())
        return {};
    return TypeSpec(TypeSpecType::Singular, lyric_common::SymbolUrl(symbolPath), typeParameters);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forSingular(
    std::initializer_list<std::string> symbolPath,
    const std::vector<TypeSpec> &typeParameters)
{
    return forSingular(lyric_common::SymbolPath(symbolPath), typeParameters);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forSingular(
    const lyric_common::SymbolUrl &symbolUrl,
    const std::vector<TypeSpec> &typeParameters)
{
    if (!symbolUrl.isValid())
        return {};
    return TypeSpec(TypeSpecType::Singular, symbolUrl, typeParameters);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forSingular(
    const tempo_utils::Url &location,
    const lyric_common::SymbolPath &path,
    const std::vector<TypeSpec> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::ModuleLocation::fromUrl(location),
        lyric_common::SymbolPath(path)),
        parameters);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forSingular(
    const tempo_utils::Url &location,
    std::initializer_list<std::string> symbolPath,
    const std::vector<TypeSpec> &parameters)
{
    return forSingular(lyric_common::SymbolUrl(
        lyric_common::ModuleLocation::fromUrl(location),
        lyric_common::SymbolPath(symbolPath)),
        parameters);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forIntersection(const std::vector<TypeSpec> &members)
{
    TU_ASSERT (!members.empty());
    return TypeSpec(TypeSpecType::Intersection, {}, members);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::forUnion(const std::vector<TypeSpec> &members)
{
    TU_ASSERT (!members.empty());
    return TypeSpec(TypeSpecType::Union, {}, members);
}

lyric_typing::TypeSpec
lyric_typing::TypeSpec::fromTypeDef(const lyric_common::TypeDef &typeDef)
{
    switch (typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            std::vector<lyric_typing::TypeSpec> parameters;
            for (auto iterator = typeDef.concreteArgumentsBegin(); iterator != typeDef.concreteArgumentsEnd(); iterator++) {
                parameters.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_typing::TypeSpec::forSingular(typeDef.getConcreteUrl(), parameters);
        }
        case lyric_common::TypeDefType::Placeholder: {
            auto templateUrl = typeDef.getPlaceholderTemplateUrl();
            auto location = templateUrl.getModuleLocation();
            auto path = templateUrl.getSymbolPath().getPath();
            auto placeholder = absl::StrCat(typeDef.getPlaceholderIndex());
            lyric_common::SymbolUrl placeholderUrl(location, lyric_common::SymbolPath(path, placeholder));
            std::vector<lyric_typing::TypeSpec> parameters;
            for (auto iterator = typeDef.placeholderArgumentsBegin(); iterator != typeDef.placeholderArgumentsEnd(); iterator++) {
                parameters.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_typing::TypeSpec::forSingular(placeholderUrl, parameters);
        }
        case lyric_common::TypeDefType::Intersection: {
            std::vector<lyric_typing::TypeSpec> members;
            for (auto iterator = typeDef.intersectionMembersBegin(); iterator != typeDef.intersectionMembersEnd(); iterator++) {
                members.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_typing::TypeSpec::forIntersection(members);
        }
        case lyric_common::TypeDefType::Union: {
            std::vector<lyric_typing::TypeSpec> members;
            for (auto iterator = typeDef.unionMembersBegin(); iterator != typeDef.unionMembersEnd(); iterator++) {
                members.push_back(TypeSpec::fromTypeDef(*iterator));
            }
            return lyric_typing::TypeSpec::forUnion(members);
        }
        default:
            return {};
    }
}

lyric_typing::TypeSpec::TypeSpec(const TypeSpec &other)
{
    m_type = other.m_type;
    m_symbolUrl = other.m_symbolUrl;
    m_parameters = other.m_parameters;
}

bool
lyric_typing::TypeSpec::isValid() const
{
    return m_type != TypeSpecType::Invalid;
}

lyric_typing::TypeSpecType
lyric_typing::TypeSpec::getType() const
{
    return m_type;
}

lyric_common::ModuleLocation
lyric_typing::TypeSpec::getTypeLocation() const
{
    return m_symbolUrl.getModuleLocation();
}

lyric_common::SymbolPath
lyric_typing::TypeSpec::getTypePath() const
{
    if (m_type != TypeSpecType::Singular)
        return {};
    return m_symbolUrl.getSymbolPath();
}

std::vector<lyric_typing::TypeSpec>
lyric_typing::TypeSpec::getTypeParameters() const
{
    if (m_type != TypeSpecType::Singular)
        return {};
    return m_parameters;
}

std::vector<lyric_typing::TypeSpec>
lyric_typing::TypeSpec::getIntersection() const
{
    if (m_type != TypeSpecType::Intersection)
        return {};
    return m_parameters;
}

std::vector<lyric_typing::TypeSpec>
lyric_typing::TypeSpec::getUnion() const
{
    if (m_type != TypeSpecType::Union)
        return {};
    return m_parameters;
}

std::string
lyric_typing::TypeSpec::toString() const
{
    switch (m_type) {
        case TypeSpecType::Singular: {
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
        case TypeSpecType::Intersection: {
            auto iterator = m_parameters.cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_parameters.cend(); iterator++) {
                string.append(" & ");
                string.append(iterator->toString());
            }
            return string;
        }
        case TypeSpecType::Union: {
            auto iterator = m_parameters.cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_parameters.cend(); iterator++) {
                string.append(" | ");
                string.append(iterator->toString());
            }
            return string;
        }
        case TypeSpecType::Invalid:
            break;
    }
    return "???";
}

bool
lyric_typing::TypeSpec::operator==(const TypeSpec &other) const
{
    return m_type == other.m_type
        && m_symbolUrl == other.m_symbolUrl
        && m_parameters == other.m_parameters;
}

bool
lyric_typing::TypeSpec::operator!=(const TypeSpec &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const lyric_typing::TypeSpec &assignable)
{
    std::forward<tempo_utils::LogMessage>(message) << "TypeSpec(" << assignable.toString() << ")";
    return std::move(message);
}