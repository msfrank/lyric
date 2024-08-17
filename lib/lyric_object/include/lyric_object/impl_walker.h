#ifndef LYRIC_OBJECT_IMPL_WALKER_H
#define LYRIC_OBJECT_IMPL_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ConceptWalker;
    class ExtensionWalker;
    class LinkWalker;
    class SymbolWalker;
    class TypeWalker;

    /**
     *
     */
    class ImplWalker {
    public:
        ImplWalker();
        ImplWalker(const ImplWalker &other);

        bool isValid() const;
        bool isDeclOnly() const;

        TypeWalker getImplType() const;

        AddressType implConceptAddressType() const;
        ConceptWalker getNearImplConcept() const;
        LinkWalker getFarImplConcept() const;

        SymbolWalker getReceiver() const;

        ExtensionWalker getExtension(tu_uint8 index) const;
        tu_uint8 numExtensions() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_implOffset;

        ImplWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 implOffset);

        friend class CallWalker;
        friend class ClassWalker;
        friend class ConceptWalker;
        friend class EnumWalker;
        friend class ExistentialWalker;
        friend class FieldWalker;
        friend class InstanceWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
        friend class StructWalker;
    };
}

#endif // LYRIC_OBJECT_IMPL_WALKER_H
