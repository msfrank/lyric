
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/member_reifier.h>

#include "lyric_typing/typing_result.h"

lyric_typing::MemberReifier::MemberReifier(TypeSystem *typeSystem)
    : m_typeSystem(typeSystem),
      m_initialized(false)
{
    TU_ASSERT (m_typeSystem != nullptr);
}

bool
lyric_typing::MemberReifier::isValid() const
{
    return m_initialized;
}

tempo_utils::Status
lyric_typing::MemberReifier::initialize(
    const lyric_common::TypeDef &receiverType,
    const lyric_assembler::TemplateHandle *templateHandle)
{
    if (!receiverType.isValid())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid receiver type", receiverType.toString());

    if (templateHandle != nullptr) {
        m_templateUrl = templateHandle->getTemplateUrl();
        m_templateParameters = templateHandle->getTemplateParameters();
        switch (receiverType.getType()) {
            case lyric_common::TypeDefType::Concrete:
                m_reifiedPlaceholders = std::vector<lyric_common::TypeDef>(
                    receiverType.concreteArgumentsBegin(), receiverType.concreteArgumentsEnd());
                break;
            case lyric_common::TypeDefType::Placeholder:
                m_reifiedPlaceholders = std::vector<lyric_common::TypeDef>(
                    receiverType.placeholderArgumentsBegin(), receiverType.placeholderArgumentsEnd());
                break;
            case lyric_common::TypeDefType::Union:
                m_reifiedPlaceholders = std::vector<lyric_common::TypeDef>(
                    receiverType.unionMembersBegin(), receiverType.unionMembersEnd());
                break;
            case lyric_common::TypeDefType::Intersection:
                m_reifiedPlaceholders = std::vector<lyric_common::TypeDef>(
                    receiverType.intersectionMembersBegin(), receiverType.intersectionMembersEnd());
                break;
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid receiver type", receiverType.toString());
        }
        if (m_reifiedPlaceholders.empty())
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "receiver {} has no placeholders", receiverType.toString());
    }

    m_initialized = true;

    return {};
}

tempo_utils::Status
lyric_typing::MemberReifier::initialize(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const std::vector<lyric_common::TypeDef> &templateArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "member reifier is already initialized");

    m_templateUrl = templateUrl;
    m_templateParameters = templateParameters;
    m_reifiedPlaceholders = templateArguments;
    m_initialized = true;

    return {};
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_typing::MemberReifier::reifyMember(
    const std::string &name,
    const lyric_assembler::FieldSymbol *fieldSymbol)
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "member reifier is not initialized");

    auto *state = m_typeSystem->getState();

    if (m_memberCache.contains(name))
        return m_memberCache[name];

    lyric_common::TypeDef reifiedType;
    auto fieldType = fieldSymbol->getTypeDef();
    switch (fieldType.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder: {
            auto reifyMemberResult = reifySingular(fieldType);
            if (reifyMemberResult.isStatus())
                return reifyMemberResult.getStatus();
            reifiedType = reifyMemberResult.getResult();
            break;
        }
        case lyric_common::TypeDefType::Union: {
            auto reifyMemberResult = reifyUnion(fieldType);
            if (reifyMemberResult.isStatus())
                return reifyMemberResult.getStatus();
            reifiedType = reifyMemberResult.getResult();
            break;
        }
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid field type", fieldType.toString());
    }

    lyric_assembler::DataReference ref;
    ref.symbolUrl = fieldSymbol->getSymbolUrl();
    ref.typeDef = reifiedType;
    ref.referenceType = fieldSymbol->isVariable()?
        lyric_assembler::ReferenceType::Variable : lyric_assembler::ReferenceType::Value;
    m_memberCache[name] = ref;

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(ref.typeDef));
    return ref;
}

tempo_utils::Status
lyric_typing::MemberReifier::checkPlaceholder(
    const lyric_object::TemplateParameter &tp,
    const lyric_common::TypeDef &arg) const
{
    if (tp.bound == lyric_object::BoundType::None)
        return {};

    auto compareAssignableResult = m_typeSystem->compareAssignable(tp.typeDef, arg);
    if (compareAssignableResult.isStatus())
        return compareAssignableResult.getStatus();
    auto comparison = compareAssignableResult.getResult();

    if (tp.bound == lyric_object::BoundType::Extends) {
        if (comparison == lyric_runtime::TypeComparison::EQUAL || comparison == lyric_runtime::TypeComparison::EXTENDS)
            return {};
    }
    if (tp.bound == lyric_object::BoundType::Super) {
        if (comparison == lyric_runtime::TypeComparison::EQUAL || comparison == lyric_runtime::TypeComparison::SUPER)
            return {};
    }

    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
        "argument type {} is not substitutable for constraint {}",
        arg.toString(), tp.typeDef.toString());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::MemberReifier::reifySingular(
    const lyric_common::TypeDef &paramType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isValid());

    lyric_common::TypeDef baseType;
    std::vector<lyric_common::TypeDef> paramTypeParameters;

    switch (paramType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if param type is concrete and takes no type parameters, we can return reified type immediately
            if (paramType.numConcreteArguments() == 0)
                return paramType;
            // otherwise set the base type to the concrete url without type parameters
            baseType = lyric_common::TypeDef::forConcrete(paramType.getConcreteUrl());
            paramTypeParameters = std::vector<lyric_common::TypeDef>(
                paramType.concreteArgumentsBegin(), paramType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (paramType.getPlaceholderTemplateUrl() == m_templateUrl) {
                auto index = paramType.getPlaceholderIndex();
                // if param type takes no parameters, we can return the reified placeholder immediately
                if (paramType.numPlaceholderArguments() == 0)
                    return m_reifiedPlaceholders[index];
                baseType = m_reifiedPlaceholders[index];
            } else {
                baseType = lyric_common::TypeDef::forPlaceholder(
                    paramType.getPlaceholderIndex(), paramType.getPlaceholderTemplateUrl());
            }
            paramTypeParameters = std::vector<lyric_common::TypeDef>(
                paramType.placeholderArgumentsBegin(), paramType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid member type", paramType.toString());
    }

    std::vector<lyric_common::TypeDef> reifiedParameters;
    for (tu_uint32 i = 0; i < paramTypeParameters.size(); i++) {
        const auto &paramTypeParameter = paramTypeParameters[i];
        lyric_common::TypeDef reifiedType;
        switch (paramTypeParameter.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                auto reifyParameterResult = reifySingular(paramTypeParameter);
                if (reifyParameterResult.isStatus())
                    return reifyParameterResult;
                reifiedType = reifyParameterResult.getResult();
                break;
            }
            case lyric_common::TypeDefType::Union: {
                auto reifyParameterResult = reifyUnion(paramTypeParameter);
                if (reifyParameterResult.isStatus())
                    return reifyParameterResult;
                reifiedType = reifyParameterResult.getResult();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "invalid type parameter {}", paramTypeParameter.toString());
        }
        reifiedParameters.push_back(reifiedType);
    }

    switch (baseType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return lyric_common::TypeDef::forConcrete(
                baseType.getConcreteUrl(), reifiedParameters);
        case lyric_common::TypeDefType::Placeholder:
            return lyric_common::TypeDef::forPlaceholder(baseType.getPlaceholderIndex(),
                baseType.getPlaceholderTemplateUrl(), reifiedParameters);
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid member type", paramType.toString());
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::MemberReifier::reifyUnion(
    const lyric_common::TypeDef &paramType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isValid());

    auto *state = m_typeSystem->getState();

    if (paramType.getType() != lyric_common::TypeDefType::Union)
        return TypingStatus::forCondition(TypingCondition::kInvalidType,
            "invalid union type", paramType.toString());
    if (paramType.numUnionMembers() == 0)
        return TypingStatus::forCondition(TypingCondition::kInvalidType,
            "invalid union type", paramType.toString());

    std::vector<lyric_common::TypeDef> unionMembers;
    for (auto iterator = paramType.unionMembersBegin(); iterator != paramType.unionMembersEnd(); iterator++) {
        auto reifyParameterResult = reifySingular(*iterator);
        if (reifyParameterResult.isStatus())
            return reifyParameterResult.getStatus();
        unionMembers.push_back(reifyParameterResult.getResult());
    }

    auto resolveUnionResult = state->typeCache()->resolveUnion(unionMembers);
    if (resolveUnionResult.isStatus())
        return resolveUnionResult.getStatus();

    return resolveUnionResult.getResult();
}
