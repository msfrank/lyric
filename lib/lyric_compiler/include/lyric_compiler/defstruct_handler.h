#ifndef LYRIC_COMPILER_DEFSTRUCT_HANDLER_H
#define LYRIC_COMPILER_DEFSTRUCT_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/struct_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "constructor_handler.h"
#include "impl_handler.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    struct DefStruct {
        lyric_assembler::StructSymbol *structSymbol = nullptr;
        lyric_assembler::StructSymbol *superstructSymbol = nullptr;
        lyric_assembler::CallSymbol *defaultCtor = nullptr;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Constructor> ctors;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Member> members;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Method> methods;
        absl::flat_hash_map<const lyric_parser::ArchetypeNode *,Impl> impls;
    };

    class DefStructHandler : public BaseGrouping {
    public:
        DefStructHandler(bool isSideEffect, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        DefStructHandler(
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
        DefStruct m_defstruct;
        std::string m_allocatorTrapName;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_currentNamespace;
    };

    class StructDefinition : public BaseChoice {
    public:
        StructDefinition(DefStruct *defstruct, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DefStruct *m_defstruct;
    };
}

#endif // LYRIC_COMPILER_DEFSTRUCT_HANDLER_H
