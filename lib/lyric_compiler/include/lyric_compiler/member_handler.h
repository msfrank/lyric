#ifndef LYRIC_COMPILER_MEMBER_HANDLER_H
#define LYRIC_COMPILER_MEMBER_HANDLER_H

#include <lyric_assembler/field_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Member {
        lyric_assembler::FieldSymbol *fieldSymbol = nullptr;
        lyric_assembler::InitializerHandle *initializerHandle = nullptr;
    };

    class MemberHandler : public BaseGrouping {
    public:
        MemberHandler(
            Member member,
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
        Member m_member;
    };

    class MemberInit : public BaseChoice {
    public:
        MemberInit(
            const lyric_common::TypeDef &memberType,
            lyric_assembler::InitializerHandle *initializerHandle,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_common::TypeDef m_memberType;
        lyric_assembler::InitializerHandle *m_initializerHandle;
    };
}

#endif // LYRIC_COMPILER_MEMBER_HANDLER_H
