
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>

lyric_assembler::TypeHandle::TypeHandle(
    const lyric_common::SymbolUrl &concreteUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments,
    TypeHandle *superType,
    ObjectState *state)
    : m_superType(superType),
      m_signature()
{
    m_typeDef = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);

//    if (m_address.isValid()) {
//        std::vector<TypeAddress> signature;
//        signature.insert(signature.cbegin(), address);
//        for (auto *super = m_superType; super != nullptr; super = super->m_superType) {
//            auto superAddress = super->getAddress();
//            TU_ASSERT (superAddress.isValid());
//            signature.insert(signature.cbegin(), superAddress);
//        }
//        m_signature = TypeSignature(signature);
//    }
}

lyric_assembler::TypeHandle::TypeHandle(
    int placeholderIndex,
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments,
    ObjectState *state)
    : m_superType(nullptr),
      m_signature()
{
    m_typeDef = lyric_common::TypeDef::forPlaceholder(placeholderIndex, templateUrl, typeArguments);

//    if (m_address.isValid()) {
//        std::vector<TypeAddress> signature;
//        signature.insert(signature.cbegin(), address);
//        // FIXME
//        for (auto *superType = m_superType; superType != nullptr; superType = superType->m_superType) {
//            auto superAddress = superType->getAddress();
//            TU_ASSERT (superAddress.isValid());
//            signature.insert(signature.cbegin(), superAddress);
//        }
//        m_signature = TypeSignature(signature);
//    }
}

lyric_assembler::TypeHandle::TypeHandle(
    const lyric_common::TypeDef &typeDef,
    TypeHandle *superType,
    ObjectState *state)
    : m_typeDef(typeDef),
      m_superType(superType),
      m_signature()
{
    TU_ASSERT (state != nullptr);
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