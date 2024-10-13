#ifndef LYRIC_COMPILER_BASE_INVOKABLE_HANDLER_H
#define LYRIC_COMPILER_BASE_INVOKABLE_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "form_handler.h"

namespace lyric_compiler {

    struct Argument {
        std::unique_ptr<lyric_assembler::CodeFragment> fragment;
        lyric_common::TypeDef resultType;
    };

    struct Invocation {
        std::vector<std::unique_ptr<Argument>> arguments;
        std::vector<tu_uint32> listOffsets;
        absl::flat_hash_map<std::string,tu_uint32> keywordOffsets;
    };

    class BaseInvokableHandler : public BaseGrouping {
    public:
        BaseInvokableHandler(
            lyric_assembler::BlockHandle *bindingBlock,
            lyric_assembler::BlockHandle *invokeBlock,
            lyric_assembler::CodeFragment *fragment,
            CompilerScanDriver *driver);

        lyric_assembler::BlockHandle *getBindingBlock() const;
        lyric_assembler::BlockHandle *getInvokeBlock() const;
        lyric_assembler::CodeFragment *getFragment() const;

        tempo_utils::Status placeArguments(
            const lyric_assembler::AbstractPlacement *placement,
            lyric_typing::CallsiteReifier &reifier,
            lyric_assembler::CodeFragment *fragment);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

    private:
        lyric_assembler::BlockHandle *m_bindingBlock;
        lyric_assembler::BlockHandle *m_invokeBlock;
        lyric_assembler::CodeFragment *m_fragment;
        Invocation m_invocation;
    };

    class InvokableArgument : public BaseChoice {
    public:
        InvokableArgument(
            Argument *argument,
            tu_uint32 offset,
            Invocation *invocation,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Argument *m_argument;
        tu_uint32 m_offset;
        Invocation *m_invocation;
    };

    class KeywordArgument : public BaseGrouping {
    public:
        KeywordArgument(
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

    private:
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_BASE_INVOKABLE_HANDLER_H
