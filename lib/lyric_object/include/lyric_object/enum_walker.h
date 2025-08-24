#ifndef LYRIC_OBJECT_ENUM_WALKER_H
#define LYRIC_OBJECT_ENUM_WALKER_H

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
    class EnumMember {
    public:
        EnumMember();
        EnumMember(const EnumMember &other);

        bool isValid() const;
        AddressType memberAddressType() const;
        FieldWalker getNearField() const;
        LinkWalker getFarField() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_enumDescriptor;
        tu_uint8 m_fieldOffset;

        EnumMember(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *enumDescriptor,
            tu_uint8 fieldOffset);

        friend class EnumWalker;
    };

    /**
     *
     */
    class EnumMethod {
    public:
        EnumMethod();
        EnumMethod(const EnumMethod &other);

        bool isValid() const;
        AddressType methodAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_enumDescriptor;
        tu_uint8 m_callOffset;

        EnumMethod(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *enumDescriptor,
            tu_uint8 callOffset);

        friend class EnumWalker;
    };

    /**
     *
     */
    class EnumWalker {
    public:
        EnumWalker();
        EnumWalker(const EnumWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        DeriveType getDeriveType() const;
        AccessType getAccess() const;

        bool hasAllocator() const;
        tu_uint32 getAllocator() const;

        CallWalker getConstructor() const;

        bool hasSuperEnum() const;
        AddressType superEnumAddressType() const;
        EnumWalker getNearSuperEnum() const;
        LinkWalker getFarSuperEnum() const;

        tu_uint8 numMembers() const;
        EnumMember getMember(tu_uint8 index) const;

        tu_uint8 numMethods() const;
        EnumMethod getMethod(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubEnums() const;
        TypeWalker getSealedSubEnum(tu_uint8 index) const;

        TypeWalker getEnumType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_enumOffset;

        EnumWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 enumOffset);

        friend class CallWalker;
        friend class FieldWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_ENUM_WALKER_H
