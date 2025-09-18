
#include <absl/strings/str_join.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::TemplateHandle::TemplateHandle(ObjectState *state)
    : m_parentBlock(nullptr), m_state(state)
{
    TU_ASSERT(m_state != nullptr);
}

lyric_assembler::TemplateHandle::TemplateHandle(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    TemplateHandle *superTemplate,
    BlockHandle *parentBlock,
    ObjectState *state)
    : TemplateHandle(templateUrl, templateParameters, parentBlock, state)
{
    m_superTemplate = superTemplate;
    TU_ASSERT (m_superTemplate != nullptr);
}

lyric_assembler::TemplateHandle::TemplateHandle(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    BlockHandle *parentBlock,
    ObjectState *state)
    : m_templateUrl(templateUrl),
      m_templateParameters(templateParameters),
      m_superTemplate(nullptr),
      m_parentBlock(parentBlock),
      m_state(state)
{
    TU_ASSERT (m_templateUrl.isValid());
    TU_ASSERT (!m_templateParameters.empty());
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);

    for (const auto &tp : m_templateParameters) {
        TU_ASSERT (tp.typeDef.isValid());
        auto placeholderType = lyric_common::TypeDef::forPlaceholder(tp.index, templateUrl).orElseThrow();
        m_placeholders.push_back(placeholderType);
        m_parameterIndex[tp.name] = tp.index;
    }
}

lyric_assembler::TemplateHandle::TemplateHandle(
    const lyric_common::SymbolUrl &templateUrl,
    TemplateHandle *superTemplate,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    ObjectState *state)
    : m_templateUrl(templateUrl),
      m_templateParameters(templateParameters),
      m_superTemplate(superTemplate),
      m_parentBlock(nullptr),
      m_state(state)
{
    TU_ASSERT (m_templateUrl.isValid());
    TU_ASSERT (!m_templateParameters.empty());
    TU_ASSERT (m_state != nullptr);

    for (const auto &tp : m_templateParameters) {
        auto placeholderType = lyric_common::TypeDef::forPlaceholder(tp.index, templateUrl).orElseThrow();
        m_placeholders.push_back(placeholderType);
        m_parameterIndex[tp.name] = tp.index;
    }
}

lyric_common::SymbolUrl
lyric_assembler::TemplateHandle::getTemplateUrl() const
{
    return m_templateUrl;
}

lyric_common::TypeDef
lyric_assembler::TemplateHandle::resolvePlaceholder(
    std::string_view name,
    const std::vector<lyric_common::TypeDef> &typeArguments) const      // NOLINT(misc-no-recursion)
{
    auto entry = m_parameterIndex.find(name);
    if (entry != m_parameterIndex.cend()) {
        if (typeArguments.empty())
            return m_placeholders[entry->second];       // use memoized instance
        auto placeholder = m_placeholders[entry->second];
        auto result = lyric_common::TypeDef::forPlaceholder(placeholder.getPlaceholderIndex(),
            placeholder.getPlaceholderTemplateUrl(), typeArguments);
        return result.orElse({});
    }
    if (m_superTemplate != nullptr)
        return m_superTemplate->resolvePlaceholder(name, typeArguments);
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TemplateHandle::resolveSingular(
    const lyric_common::SymbolPath &typePath,
    const std::vector<lyric_common::TypeDef> &typeArguments)            // NOLINT(misc-no-recursion)
{
    if (!typePath.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid type path {}", typePath.toString());

    lyric_common::TypeDef assignableType;

    // if type path is possibly a placeholder, then try to resolve the placeholder type
    if (!typePath.isEnclosed()) {
        assignableType = resolvePlaceholder(typePath.getName(), typeArguments);
    }

    // otherwise if type path is not a placeholder, then resolve the concrete type
    if (!assignableType.isValid()) {
        lyric_common::SymbolUrl concreteUrl;
        TU_ASSIGN_OR_RETURN (concreteUrl, m_parentBlock->resolveDefinition(typePath));
        TU_ASSIGN_OR_RETURN (assignableType, lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments));
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (m_state->typeCache()->getOrMakeType(assignableType));

    return assignableType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::TemplateHandle::superTemplate() const
{
    return m_superTemplate;
}

bool
lyric_assembler::TemplateHandle::hasTemplateParameter(const std::string &name) const
{
    return m_parameterIndex.contains(name);
}

lyric_object::TemplateParameter
lyric_assembler::TemplateHandle::getTemplateParameter(const std::string &name) const
{
    if (m_parameterIndex.contains(name))
        return m_templateParameters[m_parameterIndex.at(name)];
    return {};
}

lyric_object::TemplateParameter
lyric_assembler::TemplateHandle::getTemplateParameter(int index) const
{
    if (0 <= index && std::cmp_less(index, m_templateParameters.size()))
        return m_templateParameters[index];
    return {};
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::TemplateHandle::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_object::TemplateParameter>::const_iterator
lyric_assembler::TemplateHandle::templateParametersBegin() const
{
    return m_templateParameters.cbegin();
}

std::vector<lyric_object::TemplateParameter>::const_iterator
lyric_assembler::TemplateHandle::templateParametersEnd() const
{
    return m_templateParameters.cend();
}

int
lyric_assembler::TemplateHandle::numTemplateParameters() const
{
    return m_templateParameters.size();
}

lyric_common::TypeDef
lyric_assembler::TemplateHandle::getPlaceholder(int index) const
{
    if (0 <= index && std::cmp_less(index, m_placeholders.size()))
        return m_placeholders[index];
    return {};
}

lyric_common::TypeDef
lyric_assembler::TemplateHandle::getPlaceholder(const std::string &name) const
{
    if (m_parameterIndex.contains(name))
        return m_placeholders[m_parameterIndex.at(name)];
    return {};
}

std::vector<lyric_common::TypeDef>
lyric_assembler::TemplateHandle::getPlaceholders() const
{
    return m_placeholders;
}

int
lyric_assembler::TemplateHandle::numPlaceholders() const
{
    return m_placeholders.size();
}

void
lyric_assembler::TemplateHandle::touch()
{
//    for (const auto &placeholder : m_placeholders) {
//        TU_RAISE_IF_STATUS (m_state->typeCache()->getOrMakeType(placeholder));
//    }
//    m_state->typeCache()->touchTemplateParameters(m_templateParameters);
//
//    if (m_address.isValid())
//        return;
//
//    //TU_ASSERT (m_state->symbolCache()->hasSymbol(m_templateUrl));
//    lyric_assembler::AbstractSymbol *symbol;
//    TU_ASSIGN_OR_RAISE (symbol, m_state->symbolCache()->getOrImportSymbol(m_templateUrl));
//    symbol->touch();
//
//    switch (symbol->getSymbolType()) {
//        case SymbolType::EXISTENTIAL:
//            m_address = TemplateAddress(cast_symbol_to_existential(symbol)->getAddress().getAddress());
//            break;
//        case SymbolType::ACTION:
//            m_address = TemplateAddress(cast_symbol_to_action(symbol)->getAddress().getAddress());
//            break;
//        case SymbolType::CALL:
//            m_address = TemplateAddress(cast_symbol_to_call(symbol)->getAddress().getAddress());
//            break;
//        case SymbolType::CLASS:
//            m_address = TemplateAddress(cast_symbol_to_class(symbol)->getAddress().getAddress());
//            break;
//        case SymbolType::CONCEPT:
//            m_address = TemplateAddress(cast_symbol_to_concept(symbol)->getAddress().getAddress());
//            break;
//        default:
//            TU_UNREACHABLE();
//    }
}
