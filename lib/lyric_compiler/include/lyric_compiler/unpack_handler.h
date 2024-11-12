#ifndef LYRIC_COMPILER_UNPACK_HANDLER_H
#define LYRIC_COMPILER_UNPACK_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    constexpr char const *kUnwrapExtensionName = "unwrap";

    struct Unpack {
        lyric_common::TypeDef unwrapType;
        lyric_assembler::DataReference targetRef;
        std::vector<lyric_common::TypeDef> tupleTypeArguments;
        std::vector<std::pair<std::string,lyric_assembler::DataReference>> unwrapRefs;
    };

    class UnpackHandler : public BaseGrouping {
    public:
        UnpackHandler(
            const lyric_common::TypeDef &unwrapType,
            const lyric_assembler::DataReference &targetRef,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        Unpack m_unpack;
        lyric_assembler::CodeFragment *m_fragment;
    };

    class UnwrapParam : public BaseChoice {
    public:
        UnwrapParam(
            Unpack *unpack,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Unpack *m_unpack;
        lyric_assembler::CodeFragment *m_fragment;
    };
}

#endif // LYRIC_COMPILER_UNPACK_HANDLER_H
