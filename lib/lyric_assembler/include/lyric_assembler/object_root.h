#ifndef LYRIC_ASSEMBLER_OBJECT_ROOT_H
#define LYRIC_ASSEMBLER_OBJECT_ROOT_H

#include "object_state.h"

namespace lyric_assembler {

    constexpr const char *kDefaultEntryName = "$entry";

    class BlockHandle;
    class EntryHandle;

    class ObjectRoot {
    public:
        explicit ObjectRoot(ObjectState *state);
        ~ObjectRoot();

        tempo_utils::Status initialize(
            const lyric_common::ModuleLocation &preludeLocation,
            const std::vector<lyric_common::ModuleLocation> &environmentModules);

        BlockHandle *rootBlock();
        NamespaceSymbol *globalNamespace();

        bool hasEntry() const;
        lyric_common::SymbolUrl getEntry() const;
        tempo_utils::Result<EntryHandle *> declareEntry(std::string_view name = kDefaultEntryName);

    private:
        ObjectState *m_state;
        std::unique_ptr<BlockHandle> m_preludeBlock;
        std::unique_ptr<BlockHandle> m_environmentBlock;
        std::unique_ptr<BlockHandle> m_rootBlock;
        std::unique_ptr<EntryHandle> m_entryHandle;
        NamespaceSymbol *m_globalNamespace;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_ROOT_H
