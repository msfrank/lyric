#ifndef LYRIC_OBJECT_NAMESPACE_WALKER_H
#define LYRIC_OBJECT_NAMESPACE_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class LinkWalker;
    class SymbolWalker;

    /**
     *
     */
    class NamespaceWalker {
    public:
        NamespaceWalker();
        NamespaceWalker(const NamespaceWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        AccessType getAccess() const;

        bool hasSuperNamespace() const;
        AddressType superNamespaceAddressType() const;
        NamespaceWalker getNearSuperNamespace() const;
        LinkWalker getFarSuperNamespace() const;

        tu_uint32 numBindings() const;
        SymbolWalker getBinding(tu_uint32 index) const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_namespaceOffset;

        NamespaceWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 namespaceOffset);

        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_NAMESPACE_WALKER_H
