//
//#include <absl/strings/str_join.h>
//
//#include <lyric_assembler/assignable_type.h>
//#include <lyric_common/symbol_url.h>
//#include <tempo_utils/log_message.h>
//#include <tempo_utils/log_stream.h>
//
//lyric_assembler::Assignable::Assignable()
//{
//    auto priv = std::make_shared<AssignablePriv>();
//    priv->type = AssignableType::INVALID;
//    priv->symbol = lyric_common::SymbolUrl();
//    priv->parameters = std::vector<Assignable>();
//    priv->placeholder = -1;
//    m_priv = priv;
//}
//
//lyric_assembler::Assignable::Assignable(
//    AssignableType type,
//    const lyric_common::SymbolUrl &symbol,
//    const std::vector<Assignable> &parameters,
//    int placeholder)
//{
//    auto priv = std::make_shared<AssignablePriv>();
//    priv->type = type;
//    priv->symbol = symbol;
//    priv->parameters = parameters;
//    priv->placeholder = placeholder;
//    m_priv = priv;
//}
//
//lyric_assembler::Assignable::Assignable(const Assignable &other)
//    : m_priv(other.m_priv)
//{
//}
//
//lyric_assembler::Assignable::Assignable(Assignable &&other) noexcept
//{
//    m_priv = std::move(other.m_priv);
//}
//
//lyric_assembler::Assignable::~Assignable()
//{
//}
//
//lyric_assembler::Assignable &
//lyric_assembler::Assignable::operator=(const Assignable &other)
//{
//    if (this != &other) {
//        m_priv = other.m_priv;
//    }
//    return *this;
//}
//
//lyric_assembler::Assignable &
//lyric_assembler::Assignable::operator=(Assignable &&other) noexcept
//{
//    if (this != &other) {
//        m_priv = std::move(other.m_priv);
//    }
//    return *this;
//}
//
//lyric_assembler::Assignable
//lyric_assembler::Assignable::forConcrete(
//    const lyric_common::SymbolUrl &concreteUrl,
//    const std::vector<Assignable> &typeParameters)
//{
//    TU_ASSERT (concreteUrl.isValid());
//    return Assignable(AssignableType::CONCRETE, concreteUrl, typeParameters, -1);
//}
//
//lyric_assembler::Assignable
//lyric_assembler::Assignable::forPlaceholder(
//    int placeholderIndex,
//    const lyric_common::SymbolUrl &templateUrl,
//    const std::vector<Assignable> &typeParameters)
//{
//    TU_ASSERT (0 <= placeholderIndex);
//    TU_ASSERT (templateUrl.isValid());
//    return Assignable(AssignableType::PLACEHOLDER, templateUrl, typeParameters, placeholderIndex);
//}
//
//lyric_assembler::Assignable
//lyric_assembler::Assignable::forIntersection(const std::vector<Assignable> &members)
//{
//    TU_ASSERT (!members.empty());
//    return Assignable(AssignableType::INTERSECTION, {}, members, -1);
//}
//
//lyric_assembler::Assignable
//lyric_assembler::Assignable::forUnion(const std::vector<Assignable> &members)
//{
//    TU_ASSERT (!members.empty());
//    return Assignable(AssignableType::UNION, {}, members, -1);
//}
//
//bool
//lyric_assembler::Assignable::isValid() const
//{
//    return m_priv->type != AssignableType::INVALID;
//}
//
//lyric_assembler::AssignableType
//lyric_assembler::Assignable::getType() const
//{
//    return m_priv->type;
//}
//
//lyric_common::SymbolUrl
//lyric_assembler::Assignable::getConcreteUrl() const
//{
//    return m_priv->symbol;
//}
//
//int
//lyric_assembler::Assignable::getPlaceholderIndex() const
//{
//    return m_priv->placeholder;
//}
//
//lyric_common::SymbolUrl
//lyric_assembler::Assignable::getTemplateUrl() const
//{
//    return m_priv->symbol;
//}
//
//std::vector<lyric_assembler::Assignable>
//lyric_assembler::Assignable::getTypeParameters() const
//{
//    return m_priv->parameters;
//}
//
//std::vector<lyric_assembler::Assignable>
//lyric_assembler::Assignable::getIntersection() const
//{
//    return m_priv->parameters;
//}
//
//std::vector<lyric_assembler::Assignable>
//lyric_assembler::Assignable::getUnion() const
//{
//    return m_priv->parameters;
//}
//
//lyric_parser::Assignable
//lyric_assembler::Assignable::toSpec() const
//{
//    switch (m_priv->type) {
//        case AssignableType::CONCRETE: {
//            std::vector<lyric_parser::Assignable> parameters;
//            for (auto iterator = m_priv->parameters.cbegin(); iterator != m_priv->parameters.cend(); iterator++) {
//                parameters.push_back(iterator->toSpec());
//            }
//            return lyric_parser::Assignable::forSingular(m_priv->symbol, parameters);
//        }
//        case AssignableType::PLACEHOLDER: {
//            auto location = m_priv->symbol.getAssemblyLocation();
//            auto path = m_priv->symbol.getSymbolPath().getPath();
//            auto placeholder = absl::StrCat(m_priv->placeholder);
//            lyric_common::SymbolUrl placeholderUrl(location, lyric_common::SymbolPath(path, placeholder));
//            std::vector<lyric_parser::Assignable> parameters;
//            for (auto iterator = m_priv->parameters.cbegin(); iterator != m_priv->parameters.cend(); iterator++) {
//                parameters.push_back(iterator->toSpec());
//            }
//            return lyric_parser::Assignable::forSingular(placeholderUrl, parameters);
//        }
//        case AssignableType::INTERSECTION: {
//            std::vector<lyric_parser::Assignable> parameters;
//            for (auto iterator = m_priv->parameters.cbegin(); iterator != m_priv->parameters.cend(); iterator++) {
//                parameters.push_back(iterator->toSpec());
//            }
//            return lyric_parser::Assignable::forIntersection(parameters);
//        }
//        case AssignableType::UNION: {
//            std::vector<lyric_parser::Assignable> parameters;
//            for (auto iterator = m_priv->parameters.cbegin(); iterator != m_priv->parameters.cend(); iterator++) {
//                parameters.push_back(iterator->toSpec());
//            }
//            return lyric_parser::Assignable::forUnion(parameters);
//        }
//        case AssignableType::INVALID:
//            break;
//    }
//    return lyric_parser::Assignable();
//}
//
//std::string
//lyric_assembler::Assignable::toString() const
//{
//    switch (m_priv->type) {
//        case AssignableType::CONCRETE: {
//            auto string = m_priv->symbol.toString();
//            auto iterator = m_priv->parameters.cbegin();
//            if (iterator == m_priv->parameters.cend())
//                return string;
//            string.push_back('[');
//            string.append(iterator++->toString());
//            for (; iterator != m_priv->parameters.cend(); iterator++) {
//                string.append(", ");
//                string.append(iterator->toString());
//            }
//            string.push_back(']');
//            return string;
//        }
//        case AssignableType::PLACEHOLDER: {
//            auto string = absl::StrCat(m_priv->placeholder, "@", m_priv->symbol.toString());
//            auto iterator = m_priv->parameters.cbegin();
//            if (iterator == m_priv->parameters.cend())
//                return string;
//            string.push_back('[');
//            string.append(iterator++->toString());
//            for (; iterator != m_priv->parameters.cend(); iterator++) {
//                string.append(", ");
//                string.append(iterator->toString());
//            }
//            string.push_back(']');
//            return string;
//        }
//        case AssignableType::INTERSECTION: {
//            auto iterator = m_priv->parameters.cbegin();
//            auto string = iterator->toString();
//            for (++iterator; iterator != m_priv->parameters.cend(); iterator++) {
//                string.append(" & ");
//                string.append(iterator->toString());
//            }
//            return string;
//        }
//        case AssignableType::UNION: {
//            auto iterator = m_priv->parameters.cbegin();
//            auto string = iterator->toString();
//            for (++iterator; iterator != m_priv->parameters.cend(); iterator++) {
//                string.append(" | ");
//                string.append(iterator->toString());
//            }
//            return string;
//        }
//        case AssignableType::INVALID:
//            break;
//    }
//    return "???";
//}
//
//bool
//lyric_assembler::Assignable::operator==(const Assignable &other) const
//{
//    return m_priv->type == other.m_priv->type
//        && m_priv->symbol == other.m_priv->symbol
//        && m_priv->parameters == other.m_priv->parameters
//        && m_priv->placeholder == other.m_priv->placeholder;
//}
//
//bool
//lyric_assembler::Assignable::operator!=(const Assignable &other) const
//{
//    return !(*this == other);
//}
//
//tempo_utils::LogMessage&&
//lyric_assembler::operator<<(tempo_utils::LogMessage &&message, const Assignable &type)
//{
//    std::forward<tempo_utils::LogMessage>(message) << "Assignable(" << type.toString() << ")";
//    return std::move(message);
//}
