#ifndef LYRIC_COMPILER_PACK_HANDLER_H
#define LYRIC_COMPILER_PACK_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class PackHandler : public BaseGrouping {
    public:
        PackHandler(
            lyric_assembler::CallSymbol *callSymbol,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        PackHandler(
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

    private:
        lyric_assembler::CallSymbol *m_callSymbol;
    };

    class PackParam : public BaseGrouping {
    public:
        PackParam(
            lyric_assembler::CallSymbol *callSymbol,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        PackParam(
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
        lyric_assembler::CallSymbol *m_callSymbol;
        lyric_assembler::InitializerHandle *m_initializerHandle;
        lyric_assembler::Parameter m_param;
    };

    class ParamInit : public BaseChoice {
    public:
        ParamInit(
            const lyric_common::TypeDef &paramType,
            lyric_assembler::InitializerHandle *initializerHandle,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_common::TypeDef m_paramType;
        lyric_assembler::InitializerHandle *m_initializerHandle;
    };
}

#endif // LYRIC_COMPILER_PACK_HANDLER_H
