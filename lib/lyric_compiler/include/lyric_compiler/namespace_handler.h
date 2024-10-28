#ifndef LYRIC_COMPILER_NAMESPACE_HANDLER_H
#define LYRIC_COMPILER_NAMESPACE_HANDLER_H

#include <lyric_assembler/namespace_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class NamespaceHandler : public BaseGrouping {
    public:
        NamespaceHandler(
            lyric_assembler::NamespaceSymbol *parentNamespace,
            bool isSideEffect,
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
        lyric_assembler::NamespaceSymbol *m_parentNamespace;
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_namespace;
    };

    class NamespaceDefinition : public BaseChoice {
    public:
        NamespaceDefinition(
            lyric_assembler::NamespaceSymbol *namespaceSymbol,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        lyric_assembler::NamespaceSymbol *m_namespaceSymbol;
    };

}

#endif // LYRIC_COMPILER_NAMESPACE_HANDLER_H
