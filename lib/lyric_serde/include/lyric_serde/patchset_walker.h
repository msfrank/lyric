#ifndef LYRIC_SERDE_PATCHSET_WALKER_H
#define LYRIC_SERDE_PATCHSET_WALKER_H

#include <tempo_utils/integer_types.h>

#include "change_walker.h"
#include "namespace_walker.h"
#include "serde_types.h"

namespace lyric_serde {

    class PatchsetWalker {

    public:
        PatchsetWalker();
        PatchsetWalker(const PatchsetWalker &other);

        bool isValid() const;

        NamespaceWalker getNamespace(tu_uint32 index) const;
        int numNamespaces() const;

        ChangeWalker getChange(tu_uint32 index) const;
        int numChanges() const;

        ValueWalker getValue(tu_uint32 index) const;
        int numValues() const;

    private:
        std::shared_ptr<const internal::PatchsetReader> m_reader;

        PatchsetWalker(std::shared_ptr<const internal::PatchsetReader> reader);
        friend class LyricPatchset;
    };
}

#endif // LYRIC_SERDE_PATCHSET_WALKER_H