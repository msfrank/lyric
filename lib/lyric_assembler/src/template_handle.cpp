
#include <absl/strings/str_join.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::TemplateHandle::TemplateHandle(AssemblyState *state)
    : m_parentBlock(nullptr), m_state(state)
{
    TU_ASSERT(m_state != nullptr);
}

lyric_assembler::TemplateHandle::TemplateHandle(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    TemplateAddress address,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : m_templateUrl(templateUrl),
      m_templateParameters(templateParameters),
      m_address(address),
      m_parentBlock(parentBlock),
      m_state(state)
{
    TU_ASSERT (m_templateUrl.isValid());
    TU_ASSERT (!m_templateParameters.empty());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_parentBlock != nullptr);
    TU_ASSERT (m_state != nullptr);

    for (const auto &tp : m_templateParameters) {
        TU_ASSERT (tp.typeDef.isValid());
        m_placeholders.push_back(lyric_common::TypeDef::forPlaceholder(tp.index, templateUrl));
        m_parameterIndex[tp.name] = tp.index;
    }
}

lyric_assembler::TemplateHandle::TemplateHandle(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    AssemblyState *state)
    : m_templateUrl(templateUrl),
      m_templateParameters(templateParameters),
      m_parentBlock(nullptr),
      m_state(state)
{
    TU_ASSERT (m_templateUrl.isValid());
    TU_ASSERT (!m_templateParameters.empty());
    TU_ASSERT (m_state != nullptr);

    for (const auto &tp : m_templateParameters) {
        m_placeholders.push_back(lyric_common::TypeDef::forPlaceholder(tp.index, templateUrl));
        m_parameterIndex[tp.name] = tp.index;
    }
}

lyric_common::SymbolUrl
lyric_assembler::TemplateHandle::getTemplateUrl() const
{
    return m_templateUrl;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TemplateHandle::resolveSingular(
    const lyric_common::SymbolPath &typePath,
    const std::vector<lyric_common::TypeDef> &typeArguments)        // NOLINT(misc-no-recursion)
{
    if (!typePath.isValid())
        m_state->throwAssemblerInvariant("invalid type path {}", typePath.toString());

    lyric_common::TypeDef assignableType;

    // if type path is possibly a placeholder, then try to resolve the placeholder type
    if (!typePath.isEnclosed()) {
        auto name = typePath.getName();
        auto entry = m_parameterIndex.find(name);
        if (entry != m_parameterIndex.cend()) {
            if (typeArguments.empty()) {
                assignableType = m_placeholders[entry->second];     // use memoized instance
            } else {
                auto placeholder = m_placeholders[entry->second];
                assignableType = lyric_common::TypeDef::forPlaceholder(placeholder.getPlaceholderIndex(),
                    placeholder.getPlaceholderTemplateUrl(), typeArguments);
            }
        }
    }

    // otherwise if type path is not a placeholder, then resolve the concrete type
    if (!assignableType.isValid()) {
        lyric_common::SymbolUrl concreteUrl;
        TU_ASSIGN_OR_RETURN (concreteUrl, m_parentBlock->resolveDefinition(typePath));
        assignableType = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (m_state->typeCache()->getOrMakeType(assignableType));

    return assignableType;
}

lyric_assembler::TemplateAddress
lyric_assembler::TemplateHandle::getAddress()
{
    return m_address;
}

lyric_assembler::BlockHandle *
lyric_assembler::TemplateHandle::parentBlock()
{
    return m_parentBlock;
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
    for (const auto &placeholder : m_placeholders) {
        TU_RAISE_IF_STATUS (m_state->typeCache()->getOrMakeType(placeholder));
    }
    m_state->typeCache()->touchTemplateParameters(m_templateParameters);

    if (m_address.isValid())
        return;

    //TU_ASSERT (m_state->symbolCache()->hasSymbol(m_templateUrl));
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RAISE (symbol, m_state->symbolCache()->getOrImportSymbol(m_templateUrl));
    symbol->touch();

    switch (symbol->getSymbolType()) {
        case SymbolType::EXISTENTIAL:
            m_address = TemplateAddress(cast_symbol_to_existential(symbol)->getAddress().getAddress());
            break;
        case SymbolType::ACTION:
            m_address = TemplateAddress(cast_symbol_to_action(symbol)->getAddress().getAddress());
            break;
        case SymbolType::CALL:
            m_address = TemplateAddress(cast_symbol_to_call(symbol)->getAddress().getAddress());
            break;
        case SymbolType::CLASS:
            m_address = TemplateAddress(cast_symbol_to_class(symbol)->getAddress().getAddress());
            break;
        case SymbolType::CONCEPT:
            m_address = TemplateAddress(cast_symbol_to_concept(symbol)->getAddress().getAddress());
            break;
        default:
            TU_UNREACHABLE();
    }
}
