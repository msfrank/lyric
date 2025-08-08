#ifndef LYRIC_ASSEMBLER_OBJECT_ROOT_H
#define LYRIC_ASSEMBLER_OBJECT_ROOT_H

#include "object_state.h"

namespace lyric_assembler {

    class BlockHandle;

    class ObjectRoot {
    public:
        explicit ObjectRoot(ObjectState *state);

        tempo_utils::Status initialize(
            const lyric_common::ModuleLocation &preludeLocation,
            const std::vector<lyric_common::ModuleLocation> &environmentModules);

        BlockHandle *rootBlock();
        NamespaceSymbol *globalNamespace();
        CallSymbol *entryCall();

    private:
        ObjectState *m_state;
        std::unique_ptr<BlockHandle> m_preludeBlock;
        std::unique_ptr<BlockHandle> m_environmentBlock;
        std::unique_ptr<BlockHandle> m_rootBlock;
        NamespaceSymbol *m_globalNamespace;
        CallSymbol *m_entryCall;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_ROOT_H
