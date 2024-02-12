#ifndef LYRIC_SERDE_NAMESPACE_WALKER_H
#define LYRIC_SERDE_NAMESPACE_WALKER_H

#include <tempo_utils/url.h>

#include "serde_types.h"

namespace lyric_serde {

    class NamespaceWalker {

    public:
        NamespaceWalker();
        NamespaceWalker(const NamespaceWalker &other);

        bool isValid() const;

        tu_uint32 getIndex() const;

        tempo_utils::Url getUrl() const;
        std::string_view urlView() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;
        tu_uint32 m_index;

        NamespaceWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index);
        friend class PatchsetWalker;
        friend class ValueWalker;
    };
}

#endif //LYRIC_SERDE_NAMESPACE_WALKER_H
