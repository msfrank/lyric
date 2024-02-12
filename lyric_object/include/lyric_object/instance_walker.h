#ifndef LYRIC_OBJECT_INSTANCE_WALKER_H
#define LYRIC_OBJECT_INSTANCE_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class ExtensionWalker;
    class FieldWalker;
    class ImplWalker;
    class LinkWalker;
    class ParameterWalker;
    class TemplateWalker;
    class TypeWalker;

    /**
     *
     */
    class InstanceMember {
    public:
        InstanceMember();
        InstanceMember(const InstanceMember &other);

        bool isValid() const;
        //std::string getName() const;
        AddressType memberAddressType() const;
        FieldWalker getNearField() const;
        LinkWalker getFarField() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_instanceDescriptor;
        tu_uint8 m_fieldOffset;

        InstanceMember(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *instanceDescriptor,
            tu_uint8 fieldOffset);

        friend class InstanceWalker;
    };

    /**
     *
     */
    class InstanceMethod {
    public:
        InstanceMethod();
        InstanceMethod(const InstanceMethod &other);

        bool isValid() const;
        //std::string getName() const;
        AddressType methodAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_instanceDescriptor;
        tu_uint8 m_callOffset;

        InstanceMethod(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *instanceDescriptor,
            tu_uint8 callOffset);

        friend class InstanceWalker;
    };

//    /**
//     *
//     */
//    class InstanceImpl {
//    public:
//        InstanceImpl();
//        InstanceImpl(const InstanceImpl &other);
//
//        bool isValid() const;
//        TypeWalker getImplType() const;
//        ExtensionWalker getExtension(tu_uint8 index) const;
//        tu_uint8 numExtensions() const;
//
//    private:
//        std::shared_ptr<const internal::ObjectReader> m_reader;
//        void *m_instanceDescriptor;
//        tu_uint8 m_implOffset;
//
//        InstanceImpl(
//            std::shared_ptr<const internal::ObjectReader> reader,
//            void *instanceDescriptor,
//            tu_uint8 implOffset);
//
//        friend class InstanceWalker;
//    };

    /**
     *
     */
    class InstanceWalker {
    public:
        InstanceWalker();
        InstanceWalker(const InstanceWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isAbstract() const;
        bool isDeclOnly() const;
        DeriveType getDeriveType() const;

        bool hasAllocator() const;
        tu_uint32 getAllocator() const;

        CallWalker getConstructor() const;

        bool hasSuperInstance() const;
        AddressType superInstanceAddressType() const;
        InstanceWalker getNearSuperInstance() const;
        LinkWalker getFarSuperInstance() const;

        tu_uint8 numMembers() const;
        InstanceMember getMember(tu_uint8 index) const;

        tu_uint8 numMethods() const;
        InstanceMethod getMethod(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubInstances() const;
        TypeWalker getSealedSubInstance(tu_uint8 index) const;

        TypeWalker getInstanceType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_instanceOffset;

        InstanceWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 instanceOffset);

        friend class CallWalker;
        friend class FieldWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_INSTANCE_WALKER_H
