#ifndef LYRIC_COMPILER_DEFENUM_HANDLER_H
#define LYRIC_COMPILER_DEFENUM_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    struct DefEnum {
        lyric_assembler::EnumSymbol *enumSymbol = nullptr;
        lyric_assembler::EnumSymbol *superenumSymbol = nullptr;
        lyric_assembler::CallSymbol *initCall = nullptr;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Member> members;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Method> methods;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Impl> impls;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,lyric_assembler::EnumSymbol *> cases;
    };

    class DefEnumHandler : public BaseGrouping {
    public:
        DefEnumHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefEnumHandler(
            bool isSideEffect,
            lyric_assembler::NamespaceSymbol *currentNamespace,
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
        DefEnum m_defenum;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };

    class EnumDefinition : public BaseChoice {
    public:
        EnumDefinition(DefEnum *defenum, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DefEnum *m_defenum;
    };

    class EnumCase : public BaseChoice {
    public:
        EnumCase(lyric_assembler::EnumSymbol *enumcase, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_assembler::EnumSymbol *m_enumcase;
    };

    class CaseInit : public BaseInvokableHandler {
    public:
        CaseInit(
            std::unique_ptr<lyric_assembler::ConstructableInvoker> &&invoker,
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
        std::unique_ptr<lyric_assembler::ConstructableInvoker> m_invoker;
    };
}

#endif // LYRIC_COMPILER_DEFENUM_HANDLER_H