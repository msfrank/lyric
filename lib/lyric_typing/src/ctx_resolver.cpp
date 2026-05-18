
#include <lyric_typing/ctx_resolver.h>
#include <lyric_typing/typing_result.h>

lyric_typing::CtxResolver::CtxResolver(lyric_assembler::BlockHandle *block)
    : m_block(block)
{
    TU_NOTNULL (m_block);
}

tempo_utils::Result<lyric_assembler::ImplReference>
lyric_typing::CtxResolver::resolve(const lyric_common::TypeDef &ctxType)
{
    for (auto *block = m_block; block != nullptr; block = block->blockParent()) {
        auto implOption = block->getImpl(ctxType);
        if (implOption.hasValue())
            return implOption.getValue();
    }

    return TypingStatus::forCondition(TypingCondition::kTypeError,
        "no instance found implementing {}", ctxType.toString());
}