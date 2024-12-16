#ifndef LYRIC_OBJECT_BINDING_WALKER_H
#define LYRIC_OBJECT_BINDING_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class LinkWalker;
    class SymbolWalker;

    /**
     *
     */
    class BindingWalker {
    public:
        BindingWalker();
        BindingWalker(const BindingWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        AddressType bindingAddressType() const;
        SymbolWalker getNearBinding() const;
        LinkWalker getFarBinding() const;

        AccessType getAccess() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_bindingOffset;

        BindingWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 bindingOffset);

        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_BINDING_WALKER_H
