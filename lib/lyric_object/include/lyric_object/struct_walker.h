#ifndef LYRIC_OBJECT_STRUCT_WALKER_H
#define LYRIC_OBJECT_STRUCT_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class FieldWalker;
    class ImplWalker;
    class LinkWalker;
    class ParameterWalker;
    class TemplateWalker;
    class TypeWalker;

    /**
     *
     */
    class StructMember {
    public:
        StructMember();
        StructMember(const StructMember &other);

        bool isValid() const;
        AddressType memberAddressType() const;
        FieldWalker getNearField() const;
        LinkWalker getFarField() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_structDescriptor;
        tu_uint8 m_fieldOffset;

        StructMember(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *structDescriptor,
            tu_uint8 fieldOffset);

        friend class StructWalker;
    };

    /**
     *
     */
    class StructMethod {
    public:
        StructMethod();
        StructMethod(const StructMethod &other);

        bool isValid() const;
        AddressType methodAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_structDescriptor;
        tu_uint8 m_callOffset;

        StructMethod(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *structDescriptor,
            tu_uint8 callOffset);

        friend class StructWalker;
    };

    /**
     *
     */
    class StructWalker {
    public:
        StructWalker();
        StructWalker(const StructWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        DeriveType getDeriveType() const;
        AccessType getAccess() const;

        bool hasAllocator() const;
        tu_uint32 getAllocator() const;

        CallWalker getConstructor() const;

        bool hasSuperStruct() const;
        AddressType superStructAddressType() const;
        StructWalker getNearSuperStruct() const;
        LinkWalker getFarSuperStruct() const;

        tu_uint8 numMembers() const;
        StructMember getMember(tu_uint8 index) const;

        tu_uint8 numMethods() const;
        StructMethod getMethod(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubStructs() const;
        TypeWalker getSealedSubStruct(tu_uint8 index) const;

        TypeWalker getStructType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_structOffset;

        StructWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 structOffset);

        friend class CallWalker;
        friend class FieldWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_STRUCT_WALKER_H
