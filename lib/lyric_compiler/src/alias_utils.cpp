
#include <absl/strings/ascii.h>

#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_compiler/alias_utils.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>

static tempo_utils::Result<lyric_common::TypeDef>
resolve_companion(
    const lyric_common::SymbolPath &symbolPath,
    const std::string &literalValue,
    lyric_assembler::BlockHandle *block)
{
    lyric_common::TypeDef conceptType;
    TU_ASSIGN_OR_RETURN (conceptType, lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(symbolPath)));
    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_ASSIGN_OR_RETURN (conceptSymbol, block->resolveConcept(conceptType));

    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle == nullptr)
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid companion symbol '{}'", conceptType.toString());

    if (literalValue.empty())
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "invalid literal value '{}' for companion alias", literalValue);

    lyric_common::TypeDef companionType;
    if (absl::ascii_isalpha(literalValue.front())) {
        if (templateHandle->hasTemplateParameter(literalValue)) {
            companionType = templateHandle->getPlaceholder(literalValue);
        }
    } else {
        int index;
        if (!absl::SimpleAtoi(literalValue, &index))
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant,
                "invalid template parameter index {}", literalValue);
        companionType = templateHandle->getPlaceholder(index);
    }

    if (!companionType.isValid())
        return lyric_compiler::CompilerStatus::forCondition(
            lyric_compiler::CompilerCondition::kCompilerInvariant,
            "missing template parameter {} on symbol {}", literalValue, conceptType.toString());
    return companionType;
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_compiler::declare_alias(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_NOTNULL (node);
    TU_NOTNULL (block);
    TU_NOTNULL (typeSystem);

    // get binding name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get binding access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // if alias specifies a concept template parameter, then get the companion type
    lyric_common::TypeDef companionType;
    if (node->hasAttr(lyric_parser::kLyricAstSymbolPath)) {
        lyric_common::SymbolPath symbolPath;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
        std::string literalValue;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
        TU_ASSIGN_OR_RETURN (companionType, resolve_companion(symbolPath, literalValue, block));
    }

    // if binding is generic, then compile the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    if (companionType.isValid() && !templateSpec.templateParameters.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid companion alias; a companion alias cannot declare template parameters");

    // parse the target spec
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec targetSpec;
    TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // declare binding symbol in the current block only
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RETURN (bindingSymbol, block->declareBinding(identifier, isHidden, templateSpec.templateParameters));

    auto *resolver = bindingSymbol->bindingResolver();

    // define the target type
    lyric_common::TypeDef targetType;
    TU_ASSIGN_OR_RETURN (targetType, typeSystem->resolveAssignable(resolver, targetSpec));

    // set the binding target
    TU_RETURN_IF_NOT_OK (bindingSymbol->finalizeBinding(targetType, companionType));

    return bindingSymbol;
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_compiler::declare_alias(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_NOTNULL (node);
    TU_NOTNULL (currentNamespace);
    TU_NOTNULL (block);
    TU_NOTNULL (typeSystem);

    // get binding name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get binding access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    // if alias specifies a concept template parameter, then get the companion type
    lyric_common::TypeDef companionType;
    if (node->hasAttr(lyric_parser::kLyricAstSymbolPath)) {
        lyric_common::SymbolPath symbolPath;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
        std::string literalValue;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
        TU_ASSIGN_OR_RETURN (companionType, resolve_companion(symbolPath, literalValue, block));
    }

    // if binding is generic, then compile the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    if (companionType.isValid() && !templateSpec.templateParameters.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid companion alias; a companion alias cannot declare template parameters");

    // parse the target spec
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec targetSpec;
    TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // declare binding in the current namespace
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RETURN (bindingSymbol, currentNamespace->declareBinding(
        identifier, isHidden, templateSpec.templateParameters));

    auto *resolver = bindingSymbol->bindingResolver();

    // define the target type
    lyric_common::TypeDef targetType;
    TU_ASSIGN_OR_RETURN (targetType, typeSystem->resolveAssignable(resolver, targetSpec));

    // set the binding target
    TU_RETURN_IF_NOT_OK (bindingSymbol->finalizeBinding(targetType, companionType));

    return bindingSymbol;
}
