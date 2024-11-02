#ifndef LYRIC_COMPILER_DEFCLASS_HANDLER_H
#define LYRIC_COMPILER_DEFCLASS_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    struct DefClass {
        lyric_assembler::ClassSymbol *classSymbol = nullptr;
        lyric_assembler::ClassSymbol *superclassSymbol = nullptr;
        lyric_typing::TemplateSpec templateSpec;
        lyric_assembler::CallSymbol *initCall = nullptr;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Member> members;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Method> methods;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Impl> impls;
    };

    class DefClassHandler : public BaseGrouping {
    public:
        DefClassHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        DefClass m_defclass;
        bool m_isSideEffect;
    };

    class ClassDefinition : public BaseChoice {
    public:
        ClassDefinition(DefClass *defclass, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DefClass *m_defclass;
    };
}

#endif // LYRIC_COMPILER_DEFCLASS_HANDLER_H
