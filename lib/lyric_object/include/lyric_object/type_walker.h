#ifndef LYRIC_OBJECT_TYPE_WALKER_H
#define LYRIC_OBJECT_TYPE_WALKER_H

#include <lyric_common/type_def.h>

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ConcreteTypeWalker;
    class IntersectionTypeWalker;
    class PlaceholderTypeWalker;
    class UnionTypeWalker;

    class TypeWalker {

    public:
        TypeWalker();
        TypeWalker(const TypeWalker &other);

        bool isValid() const;

        bool hasSuperType() const;
        TypeWalker getSuperType() const;

        lyric_common::TypeDefType getTypeDefType() const;
        lyric_common::TypeDef getTypeDef() const;

        ConcreteTypeWalker concreteType() const;
        IntersectionTypeWalker intersectionType() const;
        PlaceholderTypeWalker placeholderType() const;
        UnionTypeWalker unionType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_typeOffset;

        TypeWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 typeOffset);

        friend class ActionWalker;
        friend class CallWalker;
        friend class ClassWalker;
        friend class ConceptWalker;
        friend class ConcreteTypeWalker;
        friend class EnumWalker;
        friend class ExistentialWalker;
        friend class FieldWalker;
        friend class ImplWalker;
        friend class InstanceImpl;
        friend class InstanceWalker;
        friend class IntersectionTypeWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class PlaceholderTypeWalker;
        friend class PlaceholderWalker;
        friend class StaticWalker;
        friend class StructWalker;
        friend class TemplateParameterWalker;
        friend class UnionTypeWalker;
    };
}

#endif // LYRIC_OBJECT_TYPE_WALKER_H
