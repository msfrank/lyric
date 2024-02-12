#ifndef LYRIC_OBJECT_STATIC_WALKER_H
#define LYRIC_OBJECT_STATIC_WALKER_H

#include <lyric_common/symbol_path.h>

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class CallWalker;
    class LinkWalker;
    class TypeWalker;

    class StaticWalker {

    public:
        StaticWalker();
        StaticWalker(const StaticWalker &other);

        bool isValid() const;

        bool isVariable() const;
        bool isDeclOnly() const;

        lyric_common::SymbolPath getSymbolPath() const;

        TypeWalker getStaticType() const;

        AddressType initializerAddressType() const;
        CallWalker getNearInitializer() const;
        LinkWalker getFarInitializer() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_staticOffset;

        StaticWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 staticOffset);

        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_STATIC_WALKER_H
