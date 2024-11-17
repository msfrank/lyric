#ifndef LYRIC_COMPILER_DEFCONCEPT_HANDLER_H
#define LYRIC_COMPILER_DEFCONCEPT_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "action_handler.h"
#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "impl_handler.h"

namespace lyric_compiler {

    struct DefConcept {
        lyric_assembler::ConceptSymbol *conceptSymbol = nullptr;
        lyric_assembler::ConceptSymbol *superconceptSymbol = nullptr;
        lyric_typing::TemplateSpec templateSpec;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Action> actions;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Impl> impls;
    };

    class DefConceptHandler : public BaseGrouping {
    public:
        DefConceptHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefConceptHandler(
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
        DefConcept m_defconcept;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };

    class ConceptDefinition : public BaseChoice {
    public:
        ConceptDefinition(DefConcept *defclass, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DefConcept *m_defconcept;
    };
}

#endif // LYRIC_COMPILER_DEFCONCEPT_HANDLER_H
