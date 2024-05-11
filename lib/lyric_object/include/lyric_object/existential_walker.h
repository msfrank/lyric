#ifndef LYRIC_OBJECT_EXISTENTIAL_WALKER_H
#define LYRIC_OBJECT_EXISTENTIAL_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class ImplWalker;
    class LinkWalker;
    class ParameterWalker;
    class TemplateWalker;
    class TypeWalker;

    /**
     *
     */
    class ExistentialMethod {
    public:
        ExistentialMethod();
        ExistentialMethod(const ExistentialMethod &other);

        bool isValid() const;
        //std::string getName() const;
        AddressType methodAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_existentialDescriptor;
        tu_uint8 m_callOffset;

        ExistentialMethod(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *existentialDescriptor,
            tu_uint8 callOffset);

        friend class ExistentialWalker;
    };

    /**
     *
     */
    class ExistentialWalker {
    public:
        ExistentialWalker();
        ExistentialWalker(const ExistentialWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        DeriveType getDeriveType() const;

        bool hasSuperExistential() const;
        AddressType superExistentialAddressType() const;
        ExistentialWalker getNearSuperExistential() const;
        LinkWalker getFarSuperExistential() const;

        bool hasTemplate() const;
        TemplateWalker getTemplate() const;

        IntrinsicType getIntrinsicType() const;

        tu_uint8 numMethods() const;
        ExistentialMethod getMethod(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubExistentials() const;
        TypeWalker getSealedSubExistential(tu_uint8 index) const;

        TypeWalker getExistentialType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_existentialOffset;

        ExistentialWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 existentialOffset);

        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_EXISTENTIAL_WALKER_H
