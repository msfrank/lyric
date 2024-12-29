
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defalias_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefAliasHandler::DefAliasHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefAliasHandler::DefAliasHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::DefAliasHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before DefAliasHandler@" << this;
    return {};
}

tempo_utils::Status
lyric_compiler::DefAliasHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after DefAliasHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefAliasClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected DefAlias node");

    // get binding name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get binding access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // if binding is generic, then compile the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    // parse the target spec
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec targetSpec;
    TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // declare binding symbol
    TU_ASSIGN_OR_RETURN (m_bindingSymbol, block->declareBinding(
        identifier, convert_access_type(access), templateSpec.templateParameters));

    auto *resolver = m_bindingSymbol->bindingResolver();

    // define the target type
    lyric_common::TypeDef targetType;
    TU_ASSIGN_OR_RETURN (targetType, typeSystem->resolveAssignable(resolver, targetSpec));
    TU_RETURN_IF_NOT_OK (m_bindingSymbol->defineTarget(targetType));

    // add global to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_bindingSymbol->getSymbolUrl()));
    }

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}
