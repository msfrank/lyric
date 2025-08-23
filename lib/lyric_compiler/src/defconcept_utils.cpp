
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defconcept_utils.h>
#include <lyric_parser/ast_attrs.h>

tempo_utils::Result<lyric_compiler::Action>
lyric_compiler::declare_concept_action(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::ConceptSymbol *conceptSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (conceptSymbol != nullptr);
    auto *conceptBlock = conceptSymbol->conceptBlock();

    // get method name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // determine the access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // parse the return type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(conceptBlock, typeNode->getArchetypeNode()));

    // parse the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(conceptBlock, packNode->getArchetypeNode()));

    Action action;

    // declare the action
    TU_ASSIGN_OR_RETURN (action.actionSymbol, conceptSymbol->declareAction(identifier, isHidden));

    TU_LOG_V << "declared method " << identifier << " for " << conceptSymbol->getSymbolUrl();

    return action;
}

tempo_utils::Result<lyric_compiler::Impl>
lyric_compiler::declare_concept_impl(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::ConceptSymbol *conceptSymbol,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (conceptSymbol != nullptr);
    auto *conceptBlock = conceptSymbol->conceptBlock();

    // parse the impl type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(conceptBlock, typeNode->getArchetypeNode()));

    // resolve the impl type
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(conceptBlock, implSpec));

    Impl impl;

    // declare the impl
    TU_ASSIGN_OR_RETURN (impl.implHandle, conceptSymbol->declareImpl(implType));

    TU_LOG_V << "declared impl " << implType << " for " << conceptSymbol->getSymbolUrl();

    return impl;
}
