
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_rewriter/assembler_attrs.h>
#include <lyric_rewriter/plugin_macro.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/compiler_schema.h>

#include "lyric_schema/assembler_schema.h"

lyric_rewriter::PluginMacro::PluginMacro()
{
}

tempo_utils::Status
lyric_rewriter::PluginMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_INFO << "rewrite PluginLocation macro";

    if (pragmaNode->numChildren() != 1)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
            "expected 1 argument for PluginLocation macro");
    auto *arg0 = pragmaNode->getChild(0);

    std::string pathString;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstLiteralValue, pathString));

    auto pluginLocation = lyric_common::ModuleLocation::fromString(pathString);
    if (!pluginLocation.isValid() || !pluginLocation.isRelative())
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
            "invalid location '{}' for PluginLocation macro", pathString);

    lyric_parser::ArchetypeNode *pluginNode;
    TU_ASSIGN_OR_RETURN (pluginNode, state->appendNode(lyric_schema::kLyricAssemblerPluginClass, {}));
    TU_RETURN_IF_NOT_OK (pluginNode->putAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation));

    return ctx.appendNode(pluginNode);
}

tempo_utils::Status
lyric_rewriter::PluginMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
        "PluginLocation macro is not valid in definition context");
}

tempo_utils::Status
lyric_rewriter::PluginMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
        "PluginLocation macro is not valid in block context");
}
