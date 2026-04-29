#ifndef LYRIC_COMPILER_GLOBAL_HANDLER_H
#define LYRIC_COMPILER_GLOBAL_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct GlobalMember {
        lyric_assembler::StaticSymbol *staticSymbol = nullptr;
        lyric_assembler::InitializerHandle *initializerHandle = nullptr;
    };

    struct GlobalMethod {
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    struct Global {
        lyric_assembler::BlockHandle *definitionBlock = nullptr;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,GlobalMember> members;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,GlobalMethod> methods;
    };

    class GlobalHandler : public BaseGrouping {
    public:
        GlobalHandler(
            Global *global,
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
        Global *m_global;
    };

    class GlobalMemberHandler : public BaseGrouping {
    public:
        GlobalMemberHandler(
            GlobalMember member,
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
        GlobalMember m_member;
    };

    class GlobalMemberInit : public BaseChoice {
    public:
        GlobalMemberInit(
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

    class GlobalMethodHandler : public BaseGrouping {
    public:
        GlobalMethodHandler(
            GlobalMethod method,
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
        GlobalMethod m_method;
    };

    tempo_utils::Status declare_global_block(
        const lyric_parser::ArchetypeNode *node,
        Global *global,
        lyric_typing::TypeSystem *typeSystem);
}

#endif // LYRIC_COMPILER_GLOBAL_HANDLER_H
