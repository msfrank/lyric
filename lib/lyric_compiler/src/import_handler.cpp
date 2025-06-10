
#include <lyric_assembler/import_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/import_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::ImportHandler::ImportHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_parentNamespace(nullptr)
{
    m_import.importBlock = block;
    TU_ASSERT (m_import.importBlock != nullptr);
}

lyric_compiler::ImportHandler::ImportHandler(
    lyric_assembler::NamespaceSymbol *parentNamespace,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(true),
      m_parentNamespace(parentNamespace)
{
    TU_ASSERT (m_parentNamespace != nullptr);
    m_import.importBlock = block;
    TU_ASSERT (m_import.importBlock != nullptr);
}

tempo_utils::Status
lyric_compiler::ImportHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // if identifier is present then declare a child namespace
    if (node->hasAttr(lyric_parser::kLyricAstIdentifier)) {
        std::string namespaceIdentifier;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, namespaceIdentifier));
        lyric_assembler::NamespaceSymbol *importNamespace;
        if (m_parentNamespace) {
            TU_ASSIGN_OR_RETURN (importNamespace, m_parentNamespace->declareSubspace(
                namespaceIdentifier, lyric_object::AccessType::Public));
            m_import.importBlock = importNamespace->namespaceBlock();
        } else {
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "cannot declare import namespace here");
        }
    }

    // resolve module to absolute location
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstImportLocation, m_import.importLocation));

    auto numChildren = node->numChildren();
    for (int i = 0; i < numChildren; i++) {
        auto symbol = std::make_unique<ImportSymbol>(&m_import, block, driver);
        ctx.appendChoice(std::move(symbol));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::ImportHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *importCache = driver->getImportCache();

    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, importCache->resolveImportLocation(m_import.importLocation));
    TU_RETURN_IF_NOT_OK (importCache->importModule(moduleLocation, m_import.importBlock, m_import.importRefs));

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::ImportSymbol::ImportSymbol(
    Import *import,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_import(import)
{
    TU_ASSERT (m_import != nullptr);
}

tempo_utils::Status
lyric_compiler::ImportSymbol::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isClass(lyric_schema::kLyricAstSymbolRefClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected SymbolRef node");

    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    m_import->importRefs.insert(lyric_assembler::ImportRef(symbolPath));

    return {};
}