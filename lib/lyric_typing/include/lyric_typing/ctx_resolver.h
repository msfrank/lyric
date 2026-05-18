#ifndef LYRIC_TYPING_CTX_RESOLVER_H
#define LYRIC_TYPING_CTX_RESOLVER_H

#include <lyric_assembler/block_handle.h>

namespace lyric_typing {

    class CtxResolver {
    public:
        explicit CtxResolver(lyric_assembler::BlockHandle *block);

        tempo_utils::Result<lyric_assembler::ImplReference> resolve(const lyric_common::TypeDef &ctxType);

    private:
        lyric_assembler::BlockHandle *m_block;
    };
}

#endif // LYRIC_TYPING_CTX_RESOLVER_H
