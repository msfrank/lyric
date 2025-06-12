#ifndef LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleSymbolOps : public BaseOps {

    public:
        explicit ModuleSymbolOps(ModuleArchetype *listener);

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
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_SYMBOL_OPS_H
