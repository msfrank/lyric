
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>

#include "lyric_assembler/linkage_symbol.h"

lyric_assembler::TypeHandle::TypeHandle(const lyric_common::TypeDef &typeDef)
    : m_typeDef(typeDef),
      m_superType(nullptr),
      m_specificSymbol(nullptr)
{
    TU_ASSERT (m_typeDef.isValid());
}

lyric_assembler::TypeHandle::TypeHandle(const lyric_common::TypeDef &typeDef, TypeHandle *superType)
    : m_typeDef(typeDef),
      m_superType(superType),
      m_specificSymbol(nullptr)
{
    TU_ASSERT (m_typeDef.isValid());
    TU_ASSERT (m_superType != nullptr);
}

lyric_assembler::TypeHandle::TypeHandle(
    const lyric_common::SymbolUrl &concreteUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments,
    TypeHandle *superType)
    : m_superType(superType),
      m_specificSymbol(nullptr)
{
    m_typeDef = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);
    TU_ASSERT (m_superType != nullptr);
}

lyric_assembler::TypeHandle::TypeHandle(
    int placeholderIndex,
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments)
    : m_superType(nullptr),
      m_specificSymbol(nullptr)
{
    m_typeDef = lyric_common::TypeDef::forPlaceholder(placeholderIndex, templateUrl, typeArguments);
}

lyric_assembler::TypeHandle::TypeHandle(
    const lyric_common::SymbolUrl &specificUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments,
    AbstractSymbol *specificSymbol)
    : m_superType(nullptr),
      m_specificSymbol(specificSymbol)
{
    m_typeDef = lyric_common::TypeDef::forConcrete(specificUrl, typeArguments);
    TU_ASSERT (m_specificSymbol != nullptr);
}

lyric_common::TypeDef
lyric_assembler::TypeHandle::getTypeDef() const
{
    return m_typeDef;
}

lyric_common::SymbolUrl
lyric_assembler::TypeHandle::getTypeSymbol() const
{
    return m_typeDef.getConcreteUrl();
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_assembler::TypeHandle::typeArgumentsBegin() const
{
    switch (m_typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return m_typeDef.concreteArgumentsBegin();
        case lyric_common::TypeDefType::Placeholder:
            return m_typeDef.placeholderArgumentsBegin();
        case lyric_common::TypeDefType::Intersection:
            return m_typeDef.intersectionMembersBegin();
        case lyric_common::TypeDefType::Union:
            return m_typeDef.unionMembersBegin();
        default:
            TU_UNREACHABLE();
    }
}

std::vector<lyric_common::TypeDef>::const_iterator
lyric_assembler::TypeHandle::typeArgumentsEnd() const
{
    switch (m_typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return m_typeDef.concreteArgumentsEnd();
        case lyric_common::TypeDefType::Placeholder:
            return m_typeDef.placeholderArgumentsEnd();
        case lyric_common::TypeDefType::Intersection:
            return m_typeDef.intersectionMembersEnd();
        case lyric_common::TypeDefType::Union:
            return m_typeDef.unionMembersEnd();
        default:
            TU_UNREACHABLE();
    }
}

lyric_assembler::TypeHandle *
lyric_assembler::TypeHandle::getSuperType() const
{
    return m_superType;
}

tempo_utils::Status
lyric_assembler::TypeHandle::defineType(
    const std::vector<lyric_common::TypeDef> &typeArguments,
    TypeHandle *superType)
{
    if (m_specificSymbol == nullptr || m_specificSymbol->getSymbolType() != SymbolType::TYPENAME)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "type {} is already defined", m_typeDef.toString());

    auto concreteUrl = m_typeDef.getConcreteUrl();
    m_typeDef = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);
    m_superType = superType;
    m_specificSymbol = nullptr;
    return {};
}