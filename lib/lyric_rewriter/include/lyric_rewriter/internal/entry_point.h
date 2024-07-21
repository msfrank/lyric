#ifndef LYRIC_REWRITER_INTERNAL_ENTRY_POINT_H
#define LYRIC_REWRITER_INTERNAL_ENTRY_POINT_H

#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_common/symbol_url.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_typing/type_system.h>

#include "../macro_registry.h"

namespace lyric_rewriter::internal {

    class EntryPoint {

    public:
        EntryPoint(MacroRegistry *registry, lyric_parser::ArchetypeState *state);
        ~EntryPoint();

        MacroRegistry *getRegistry() const;
        lyric_parser::ArchetypeState *getState() const;

    private:
        MacroRegistry *m_registry;
        lyric_parser::ArchetypeState *m_state;
    };
}

#endif // LYRIC_REWRITER_INTERNAL_ENTRY_POINT_H