
#include <absl/strings/str_join.h>

#include <lyric_common/symbol_url.h>
#include <lyric_common/type_def.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/log_stream.h>

lyric_common::TypeDef::TypeDef()
    : m_priv(std::make_shared<Priv>(TypeDefType::Invalid, lyric_common::SymbolUrl{}, std::vector<TypeDef>{}, -1))
{
}

lyric_common::TypeDef::TypeDef(
    TypeDefType type,
    const SymbolUrl &symbol,
    const std::vector<TypeDef> &parameters,
    int placeholder)
    : m_priv(std::make_shared<Priv>(type, symbol, parameters, placeholder))
{
}

lyric_common::TypeDef::TypeDef(const TypeDef &other)
    : m_priv(other.m_priv)
{
}

lyric_common::TypeDef::TypeDef(TypeDef &&other) noexcept
{
    m_priv = std::move(other.m_priv);
}

lyric_common::TypeDef &
lyric_common::TypeDef::operator=(const TypeDef &other)
{
    if (this != &other) {
        m_priv = other.m_priv;
    }
    return *this;
}

lyric_common::TypeDef &
lyric_common::TypeDef::operator=(TypeDef &&other) noexcept
{
    if (this != &other) {
        m_priv = std::move(other.m_priv);
    }
    return *this;
}

lyric_common::TypeDef
lyric_common::TypeDef::forConcrete(
    const SymbolUrl &concreteUrl,
    const std::vector<TypeDef> &typeParameters)
{
    TU_ASSERT (concreteUrl.isValid());
    return TypeDef(TypeDefType::Concrete, concreteUrl, typeParameters, -1);
}

lyric_common::TypeDef
lyric_common::TypeDef::forPlaceholder(
    int placeholderIndex,
    const SymbolUrl &templateUrl,
    const std::vector<TypeDef> &typeParameters)
{
    TU_ASSERT (0 <= placeholderIndex);
    TU_ASSERT (templateUrl.isValid());
    return TypeDef(TypeDefType::Placeholder, templateUrl, typeParameters, placeholderIndex);
}

lyric_common::TypeDef
lyric_common::TypeDef::forIntersection(const std::vector<TypeDef> &members)
{
    TU_ASSERT (!members.empty());
    return TypeDef(TypeDefType::Intersection, {}, members, -1);
}

lyric_common::TypeDef
lyric_common::TypeDef::forUnion(const std::vector<TypeDef> &members)
{
    TU_ASSERT (!members.empty());
    return TypeDef(TypeDefType::Union, {}, members, -1);
}

lyric_common::TypeDef
lyric_common::TypeDef::noReturn()
{
    return TypeDef(TypeDefType::NoReturn, {}, {}, -1);
}

bool
lyric_common::TypeDef::isValid() const
{
    return m_priv->type != TypeDefType::Invalid;
}

lyric_common::TypeDefType
lyric_common::TypeDef::getType() const
{
    return m_priv->type;
}

lyric_common::SymbolUrl
lyric_common::TypeDef::getConcreteUrl() const
{
    if (m_priv->type == TypeDefType::Concrete)
        return m_priv->symbol;
    return {};
}

std::span<const lyric_common::TypeDef>
lyric_common::TypeDef::getConcreteArguments() const
{
    if (m_priv->type == TypeDefType::Concrete)
        return *m_priv->parameters;
    return {};
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::concreteArgumentsBegin() const
{
    if (m_priv->type == TypeDefType::Concrete)
        return m_priv->parameters->cbegin();
    return m_priv->parameters->cend();
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::concreteArgumentsEnd() const
{
    return m_priv->parameters->cend();
}

int
lyric_common::TypeDef::numConcreteArguments() const
{
    if (m_priv->type == TypeDefType::Concrete)
        return m_priv->parameters->size();
    return 0;
}

int
lyric_common::TypeDef::getPlaceholderIndex() const
{
    if (m_priv->type == TypeDefType::Placeholder)
        return m_priv->placeholder;
    return -1;
}

lyric_common::SymbolUrl
lyric_common::TypeDef::getPlaceholderTemplateUrl() const
{
    if (m_priv->type == TypeDefType::Placeholder)
        return m_priv->symbol;
    return {};
}

std::span<const lyric_common::TypeDef>
lyric_common::TypeDef::getPlaceholderArguments() const
{
    if (m_priv->type == TypeDefType::Placeholder)
        return *m_priv->parameters;
    return {};
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::placeholderArgumentsBegin() const
{
    if (m_priv->type == TypeDefType::Placeholder)
        return m_priv->parameters->cbegin();
    return m_priv->parameters->cend();
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::placeholderArgumentsEnd() const
{
    return m_priv->parameters->cend();
}

int
lyric_common::TypeDef::numPlaceholderArguments() const
{
    if (m_priv->type == TypeDefType::Placeholder)
        return m_priv->parameters->size();
    return 0;
}

std::span<const lyric_common::TypeDef>
lyric_common::TypeDef::getIntersectionMembers() const
{
    if (m_priv->type == TypeDefType::Intersection)
        return *m_priv->parameters;
    return {};
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::intersectionMembersBegin() const
{
    if (m_priv->type == TypeDefType::Intersection)
        return m_priv->parameters->cbegin();
    return m_priv->parameters->cend();
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::intersectionMembersEnd() const
{
    return m_priv->parameters->cend();
}

int
lyric_common::TypeDef::numIntersectionMembers() const
{
    if (m_priv->type == TypeDefType::Intersection)
        return m_priv->parameters->size();
    return 0;
}

std::span<const lyric_common::TypeDef>
lyric_common::TypeDef::getUnionMembers() const
{
    if (m_priv->type == TypeDefType::Union)
        return *m_priv->parameters;
    return {};
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::unionMembersBegin() const
{
    if (m_priv->type == TypeDefType::Union)
        return m_priv->parameters->cbegin();
    return m_priv->parameters->cend();
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_common::TypeDef::unionMembersEnd() const
{
    return m_priv->parameters->cend();
}

int
lyric_common::TypeDef::numUnionMembers() const
{
    if (m_priv->type == TypeDefType::Union)
        return m_priv->parameters->size();
    return 0;
}

std::string
lyric_common::TypeDef::toString() const
{
    switch (m_priv->type) {
        case TypeDefType::Concrete: {
            auto string = m_priv->symbol.toString();
            auto iterator = m_priv->parameters->cbegin();
            if (iterator == m_priv->parameters->cend())
                return string;
            string.push_back('[');
            string.append(iterator++->toString());
            for (; iterator != m_priv->parameters->cend(); iterator++) {
                string.append(", ");
                string.append(iterator->toString());
            }
            string.push_back(']');
            return string;
        }
        case TypeDefType::Placeholder: {
            auto string = absl::StrCat(m_priv->placeholder, "@", m_priv->symbol.toString());
            auto iterator = m_priv->parameters->cbegin();
            if (iterator == m_priv->parameters->cend())
                return string;
            string.push_back('[');
            string.append(iterator++->toString());
            for (; iterator != m_priv->parameters->cend(); iterator++) {
                string.append(", ");
                string.append(iterator->toString());
            }
            string.push_back(']');
            return string;
        }
        case TypeDefType::Intersection: {
            auto iterator = m_priv->parameters->cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_priv->parameters->cend(); iterator++) {
                string.append(" & ");
                string.append(iterator->toString());
            }
            return string;
        }
        case TypeDefType::Union: {
            auto iterator = m_priv->parameters->cbegin();
            auto string = iterator->toString();
            for (++iterator; iterator != m_priv->parameters->cend(); iterator++) {
                string.append(" | ");
                string.append(iterator->toString());
            }
            return string;
        }
        case TypeDefType::NoReturn: {
            return "(No Return)";
        }
        case TypeDefType::Invalid:
            break;
    }
    return "???";
}

bool
lyric_common::TypeDef::operator==(const TypeDef &other) const
{
    return m_priv->type == other.m_priv->type
        && m_priv->placeholder == other.m_priv->placeholder
        && m_priv->symbol == other.m_priv->symbol
        && m_priv->parameters == other.m_priv->parameters;
}

bool
lyric_common::TypeDef::operator!=(const TypeDef &other) const
{
    return !(*this == other);
}

tempo_utils::LogMessage&&
lyric_common::operator<<(tempo_utils::LogMessage &&message, const TypeDef &type)
{
    std::forward<tempo_utils::LogMessage>(message) << "TypeDef(" << type.toString() << ")";
    return std::move(message);
}
