
#include <absl/strings/str_join.h>

#include <lyric_assembler/binding_symbol.h>
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

    lyric_common::TypeDef typeDef;

    // if type path is possibly a placeholder, then try to resolve the placeholder type
    if (!typePath.isEnclosed()) {
        typeDef = resolvePlaceholder(typePath.getName(), typeArguments);
    }

    // otherwise if type path is not a placeholder, then resolve the concrete type
    if (!typeDef.isValid()) {
        lyric_common::SymbolUrl concreteUrl;
        TU_ASSIGN_OR_RETURN (concreteUrl, m_parentBlock->resolveDefinition(typePath));
        TU_ASSIGN_OR_RETURN (typeDef, lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments));

        auto *symbolCache = m_state->symbolCache();

        AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(concreteUrl));

        TemplateHandle *templateHandle = nullptr;
        switch (symbol->getSymbolType()) {

            // if symbol could be generic then get the template
            case SymbolType::CLASS:
                templateHandle = cast_symbol_to_class(symbol)->classTemplate();
                break;
            case SymbolType::CONCEPT:
                templateHandle = cast_symbol_to_concept(symbol)->conceptTemplate();
                break;
            case SymbolType::EXISTENTIAL:
                templateHandle = cast_symbol_to_existential(symbol)->existentialTemplate();
                break;

                // special case: if symbol is a binding then resolve the target
            case SymbolType::BINDING: {
                auto *bindingSymbol = cast_symbol_to_binding(symbol);
                TU_ASSIGN_OR_RETURN (typeDef, bindingSymbol->resolveTarget(typeArguments));
                break;
            }

                // otherwise we don't need to perform further validation
            default:
                break;
        }

        // if symbol is generic then validate the number of type arguments matches the template
        if (templateHandle != nullptr) {
            if (templateHandle->numTemplateParameters() != typeArguments.size())
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kIncompatibleType,
                    "wrong number of type arguments for {}; expected {} but found {}",
                    typeDef.toString(), templateHandle->numTemplateParameters(), typeArguments.size());
        }
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (m_state->typeCache()->getOrMakeType(typeDef));

    return typeDef;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::TemplateHandle::resolveDefinition(const lyric_common::SymbolPath &symbolPath)
{
    return m_parentBlock->resolveDefinition(symbolPath);
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