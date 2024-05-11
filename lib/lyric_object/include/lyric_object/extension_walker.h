#ifndef LYRIC_OBJECT_EXTENSION_WALKER_H
#define LYRIC_OBJECT_EXTENSION_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ActionWalker;
    class CallWalker;
    class LinkWalker;

    /**
     *
     */
    class ExtensionWalker {
    public:
        ExtensionWalker();
        ExtensionWalker(const ExtensionWalker &other);

        bool isValid() const;

        //std::string getName() const;

        AddressType actionAddressType() const;
        ActionWalker getNearAction() const;
        LinkWalker getFarAction() const;

        AddressType callAddressType() const;
        CallWalker getNearCall() const;
        LinkWalker getFarCall() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_implDescriptor;
        tu_uint8 m_extensionOffset;

        ExtensionWalker(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *implDescriptor,
            tu_uint8 extensionOffset);

        friend class ImplWalker;
    };
}

#endif // LYRIC_OBJECT_EXTENSION_WALKER_H
