#ifndef LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleSymbolOps {

    public:
        explicit ModuleSymbolOps(ArchetypeState *state);
        virtual ~ModuleSymbolOps() = default;

        void enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx);
        void exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx);
        void exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx);

        void enterUsingStatement(ModuleParser::UsingStatementContext *ctx);
        void exitUsingRef(ModuleParser::UsingRefContext *ctx);
        void exitUsingType(ModuleParser::UsingTypeContext *ctx);
        void exitUsingStatement(ModuleParser::UsingStatementContext *ctx);

        void exitImportRef(ModuleParser::ImportRefContext *ctx);
        void exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx);
        void exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx);
        void enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx);
        void exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H
