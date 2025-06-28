
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/defstatic_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefStaticHandler::DefStaticHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefStaticHandler::DefStaticHandler(
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
lyric_compiler::DefStaticHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before DefStaticHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefStaticClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected DefStatic node");

    // get global name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get global access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // determine whether global is variable
    bool isVariable;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsVariable, isVariable));

    // resolve the declaration type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec declarationSpec;
    TU_ASSIGN_OR_RETURN (declarationSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    lyric_common::TypeDef declarationType;
    TU_ASSIGN_OR_RETURN (declarationType, typeSystem->resolveAssignable(block, declarationSpec));

    // declare static symbol
    lyric_assembler::DataReference staticRef;
    TU_ASSIGN_OR_RETURN (staticRef, block->declareStatic(
        identifier, convert_access_type(access), declarationType, isVariable));

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(staticRef.symbolUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::STATIC)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid static symbol {}", staticRef.symbolUrl.toString());
    m_staticSymbol = cast_symbol_to_static(symbol);

    // add global to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_staticSymbol->getSymbolUrl()));
    }

    //
    TU_ASSIGN_OR_RETURN (m_initializerHandle, m_staticSymbol->defineInitializer());
    auto *procHandle = m_initializerHandle->initializerProc();
    auto *code = procHandle->procCode();
    auto *fragment = code->rootFragment();

    auto expression = std::make_unique<FormChoice>(
        FormType::Expression, fragment, block, driver);
    ctx.appendChoice(std::move(expression));

    return {};
}

tempo_utils::Status
lyric_compiler::DefStaticHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after DefStaticHandler@" << this;

    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *procHandle = m_initializerHandle->initializerProc();
    auto *code = procHandle->procCode();
    auto *fragment = code->rootFragment();

    auto initializerType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    procHandle->putExitType(initializerType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_initializerHandle->finalizeInitializer());

    auto declarationType = m_staticSymbol->getTypeDef();
    bool isAssignable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(declarationType, returnType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "static initializer is incompatible with type {}", declarationType.toString());

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}
