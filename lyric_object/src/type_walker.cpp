
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/intersection_type_walker.h>
#include <lyric_object/placeholder_type_walker.h>
#include <lyric_object/type_walker.h>
#include <lyric_object/union_type_walker.h>

lyric_object::TypeWalker::TypeWalker()
    : m_typeOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::TypeWalker::TypeWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 typeOffset)
    : m_reader(reader),
      m_typeOffset(typeOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_typeOffset != INVALID_ADDRESS_U32);
}

lyric_object::TypeWalker::TypeWalker(const TypeWalker &other)
    : m_reader(other.m_reader),
      m_typeOffset(other.m_typeOffset)
{
}

bool
lyric_object::TypeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_typeOffset < m_reader->numTypes();
}

bool
lyric_object::TypeWalker::hasSuperType() const
{
    if (!isValid())
        return false;
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return false;
    return typeDescriptor->super_type() != INVALID_ADDRESS_U32;
}

lyric_object::TypeWalker
lyric_object::TypeWalker::getSuperType() const
{
    if (!isValid())
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, typeDescriptor->super_type());
}

lyric_common::TypeDefType
lyric_object::TypeWalker::getTypeDefType() const
{
    if (!isValid())
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return lyric_common::TypeDefType::Invalid;

    switch (typeDescriptor->type_assignable_type()) {
        case lyo1::Assignable::ConcreteAssignable:
            return lyric_common::TypeDefType::Concrete;
        case lyo1::Assignable::IntersectionAssignable:
            return lyric_common::TypeDefType::Intersection;
        case lyo1::Assignable::PlaceholderAssignable:
            return lyric_common::TypeDefType::Placeholder;
        case lyo1::Assignable::UnionAssignable:
            return lyric_common::TypeDefType::Union;
        case lyo1::Assignable::SpecialAssignable: {
            switch (typeDescriptor->type_assignable_as_SpecialAssignable()->special_type()) {
                case lyo1::SpecialType::NoReturn:
                    return lyric_common::TypeDefType::NoReturn;
                default:
                    return lyric_common::TypeDefType::Invalid;
            }
        }
        default:
            return lyric_common::TypeDefType::Invalid;
    }
}

lyric_common::TypeDef
lyric_object::TypeWalker::getTypeDef() const
{
    if (!isValid())
        return {};

    auto type = getTypeDefType();

    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};

    switch (type) {
        case lyric_common::TypeDefType::Concrete: {
            ConcreteTypeWalker walker(m_reader, (void *) typeDescriptor->type_assignable_as_ConcreteAssignable());
            return walker.getTypeDef();
        }
        case lyric_common::TypeDefType::Intersection: {
            IntersectionTypeWalker walker(
                m_reader, (void *) typeDescriptor->type_assignable_as_IntersectionAssignable());
            return walker.getTypeDef();
        }
        case lyric_common::TypeDefType::Placeholder: {
            PlaceholderTypeWalker walker(
                m_reader, (void *) typeDescriptor->type_assignable_as_PlaceholderAssignable());
            return walker.getTypeDef();
        }
        case lyric_common::TypeDefType::Union: {
            UnionTypeWalker walker(m_reader, (void *) typeDescriptor->type_assignable_as_UnionAssignable());
            return walker.getTypeDef();
        }
        default:
            return {};
    }
}

lyric_object::ConcreteTypeWalker
lyric_object::TypeWalker::concreteType() const
{
    if (getTypeDefType() != lyric_common::TypeDefType::Concrete)
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};
    return ConcreteTypeWalker(m_reader, (void *) typeDescriptor->type_assignable_as_ConcreteAssignable());
}

lyric_object::IntersectionTypeWalker
lyric_object::TypeWalker::intersectionType() const
{
    if (getTypeDefType() != lyric_common::TypeDefType::Intersection)
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};
    return IntersectionTypeWalker(m_reader, (void *) typeDescriptor->type_assignable_as_IntersectionAssignable());
}

lyric_object::PlaceholderTypeWalker
lyric_object::TypeWalker::placeholderType() const
{
    if (getTypeDefType() != lyric_common::TypeDefType::Placeholder)
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};
    return PlaceholderTypeWalker(m_reader, (void *) typeDescriptor->type_assignable_as_PlaceholderAssignable());
}

lyric_object::UnionTypeWalker
lyric_object::TypeWalker::unionType() const
{
    if (getTypeDefType() != lyric_common::TypeDefType::Union)
        return {};
    auto *typeDescriptor = m_reader->getType(m_typeOffset);
    if (typeDescriptor == nullptr)
        return {};
    return UnionTypeWalker(m_reader, (void *) typeDescriptor->type_assignable_as_UnionAssignable());
}

tu_uint32
lyric_object::TypeWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_typeOffset;
}
