#ifndef LYRIC_OBJECT_SYMBOL_WALKER_H
#define LYRIC_OBJECT_SYMBOL_WALKER_H

#include <tempo_utils/integer_types.h>

#include "object_types.h"

namespace lyric_object {

    class SymbolWalker {

    public:
        SymbolWalker();
        SymbolWalker(const SymbolWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;
        LinkageSection getLinkageSection() const;
        tu_uint32 getLinkageIndex() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_symbolOffset;

        SymbolWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 symbolOffset);

        friend class ActionWalker;
        friend class BindingWalker;
        friend class CallWalker;
        friend class ImplWalker;
        friend class NamespaceWalker;
        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_SYMBOL_WALKER_H
