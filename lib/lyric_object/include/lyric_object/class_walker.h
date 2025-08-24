#ifndef LYRIC_OBJECT_CLASS_WALKER_H
#define LYRIC_OBJECT_CLASS_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ActionWalker;
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
    class ClassMember {
    public:
        ClassMember();
        ClassMember(const ClassMember &other);

        bool isValid() const;
        AddressType memberAddressType() const;
        FieldWalker getNearField() const;
        LinkWalker getFarField() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_classDescriptor;
        tu_uint8 m_fieldOffset;

        ClassMember(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *classDescriptor,
            tu_uint8 fieldOffset);

        friend class ClassWalker;
    };

    /**
     *
     */
    class ClassMethod {
    public:
        ClassMethod();
        ClassMethod(const ClassMethod &other);

        bool isValid() const;
        AddressType methodAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_classDescriptor;
        tu_uint8 m_callOffset;

        ClassMethod(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *classDescriptor,
            tu_uint8 callOffset);

        friend class ClassWalker;
    };

    /**
     *
     */
    class ClassWalker {
    public:
        ClassWalker();
        ClassWalker(const ClassWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        DeriveType getDeriveType() const;
        AccessType getAccess() const;

        bool hasAllocator() const;
        tu_uint32 getAllocator() const;

        CallWalker getConstructor() const;

        bool hasSuperClass() const;
        AddressType superClassAddressType() const;
        ClassWalker getNearSuperClass() const;
        LinkWalker getFarSuperClass() const;

        bool hasTemplate() const;
        TemplateWalker getTemplate() const;

        tu_uint8 numMembers() const;
        ClassMember getMember(tu_uint8 index) const;

        tu_uint8 numMethods() const;
        ClassMethod getMethod(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubClasses() const;
        TypeWalker getSealedSubClass(tu_uint8 index) const;

        TypeWalker getClassType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_classOffset;

        ClassWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 classOffset);

        friend class CallWalker;
        friend class FieldWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_CLASS_WALKER_H
