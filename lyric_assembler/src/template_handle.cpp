
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
lyric_assembler::TemplateHandle::resolveSingular(const lyric_parser::Assignable &assignableSpec)     // NOLINT(misc-no-recursion)
{
    if (assignableSpec.getType() != lyric_parser::AssignableType::SINGULAR)
        m_state->throwAssemblerInvariant(
            "attempted to resolve assignable for non singular type {}", assignableSpec.toString());

    std::vector<lyric_common::TypeDef> typeArguments;
    for (const auto &parameter : assignableSpec.getTypeParameters()) {
        auto resolveParameterResult = resolveAssignable(parameter);
        if (resolveParameterResult.isStatus())
            return resolveParameterResult;
        typeArguments.push_back(resolveParameterResult.getResult());
    }

    lyric_common::TypeDef assignableType;

    // if type path is a placeholder, then resolve the placeholder type
    if (!m_parameterIndex.empty()) {
        auto name = absl::StrJoin(assignableSpec.getTypePath().getPath(), ".");
        if (m_parameterIndex.contains(name)) {
            if (typeArguments.empty()) {
                assignableType = m_placeholders[m_parameterIndex[name]];    // use memoized instance
            } else {
                auto placeholder = m_placeholders[m_parameterIndex[name]];
                assignableType = lyric_common::TypeDef::forPlaceholder(placeholder.getPlaceholderIndex(),
                    placeholder.getPlaceholderTemplateUrl(), typeArguments);
            }
        }
    }

    // otherwise if type path is not a placeholder, then resolve the concrete type
    if (!assignableType.isValid()) {
        auto resolveTypePathResult = m_parentBlock->resolveDefinition(assignableSpec.getTypePath());
        if (resolveTypePathResult.isStatus())
            return resolveTypePathResult.getStatus();
        assignableType = lyric_common::TypeDef::forConcrete(resolveTypePathResult.getResult(), typeArguments);
    }

    // if there is no type handle for type, then create it
    if (!m_state->typeCache()->hasType(assignableType)) {
        m_state->typeCache()->makeType(assignableType);
    }

    return assignableType;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TemplateHandle::resolveAssignable(const lyric_parser::Assignable &assignableSpec)
{
    if (assignableSpec.getType() == lyric_parser::AssignableType::SINGULAR)
        return resolveSingular(assignableSpec);

    if (assignableSpec.getType() == lyric_parser::AssignableType::UNION) {
        std::vector<lyric_common::TypeDef> unionMembers;
        for (const auto &memberSpec : assignableSpec.getUnion()) {
            auto resolveMemberResult = resolveSingular(memberSpec);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult;
            unionMembers.push_back(resolveMemberResult.getResult());
        }
        return m_state->typeCache()->resolveUnion(unionMembers);
    }
    if (assignableSpec.getType() == lyric_parser::AssignableType::INTERSECTION) {
        std::vector<lyric_common::TypeDef> intersectionMembers;
        for (const auto &memberSpec : assignableSpec.getIntersection()) {
            auto resolveMemberResult = resolveSingular(memberSpec);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult;
            intersectionMembers.push_back(resolveMemberResult.getResult());
        }
        return m_state->typeCache()->resolveIntersection(intersectionMembers);
    }

    m_state->throwAssemblerInvariant("failed to resolve non singular base type {}", assignableSpec.toString());
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
        m_state->typeCache()->touchType(placeholder);
    }
    m_state->typeCache()->touchTemplateParameters(m_templateParameters);

    if (m_address.isValid())
        return;

    TU_ASSERT (m_state->symbolCache()->hasSymbol(m_templateUrl));
    auto *sym = m_state->symbolCache()->getSymbol(m_templateUrl);
    sym->touch();

    switch (sym->getSymbolType()) {
        case SymbolType::EXISTENTIAL:
            m_address = TemplateAddress(cast_symbol_to_existential(sym)->getAddress().getAddress());
            break;
        case SymbolType::ACTION:
            m_address = TemplateAddress(cast_symbol_to_action(sym)->getAddress().getAddress());
            break;
        case SymbolType::CALL:
            m_address = TemplateAddress(cast_symbol_to_call(sym)->getAddress().getAddress());
            break;
        case SymbolType::CLASS:
            m_address = TemplateAddress(cast_symbol_to_class(sym)->getAddress().getAddress());
            break;
        case SymbolType::CONCEPT:
            m_address = TemplateAddress(cast_symbol_to_concept(sym)->getAddress().getAddress());
            break;
        default:
            TU_UNREACHABLE();
    }
}
