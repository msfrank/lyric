#ifndef LYRIC_ARCHIVER_SYMBOL_REFERENCE_SET_H
#define LYRIC_ARCHIVER_SYMBOL_REFERENCE_SET_H

#include <absl/container/flat_hash_set.h>

#include <lyric_common/symbol_url.h>

#include "archiver_result.h"

namespace lyric_archiver {

    class ArchiverState;

    class SymbolReferenceSet {
    public:
        explicit SymbolReferenceSet(ArchiverState *state);

        bool isEmpty() const;
        tempo_utils::Status insertReference(const lyric_common::SymbolUrl &symbolUrl);
        tempo_utils::Result<lyric_common::SymbolUrl> takeReference();

    private:
        ArchiverState *m_state;
        absl::flat_hash_set<lyric_common::SymbolUrl> m_copyReferences;
    };
}

#endif // LYRIC_ARCHIVER_SYMBOL_REFERENCE_SET_H
