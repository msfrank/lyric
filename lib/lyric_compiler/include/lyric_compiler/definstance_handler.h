#ifndef LYRIC_COMPILER_DEFINSTANCE_HANDLER_H
#define LYRIC_COMPILER_DEFINSTANCE_HANDLER_H

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

    struct DefInstance {
        lyric_assembler::InstanceSymbol *instanceSymbol = nullptr;
        lyric_assembler::InstanceSymbol *superinstanceSymbol = nullptr;
        lyric_assembler::CallSymbol *initCall = nullptr;
        bool defaultInit = false;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Member> members;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Method> methods;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Impl> impls;
    };

    class DefInstanceHandler : public BaseGrouping {
    public:
        DefInstanceHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefInstanceHandler(
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
        DefInstance m_definstance;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };

    class InstanceDefinition : public BaseChoice {
    public:
        InstanceDefinition(DefInstance *definstance, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DefInstance *m_definstance;
    };
}

#endif // LYRIC_COMPILER_DEFINSTANCE_HANDLER_H
