
#include <lyric_assembler/object_root.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/alias_handler.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/defconcept_handler.h>
#include <lyric_compiler/defenum_handler.h>
#include <lyric_compiler/definstance_handler.h>
#include <lyric_compiler/defstatic_handler.h>
#include <lyric_compiler/defstruct_handler.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_compiler/import_handler.h>
#include <lyric_compiler/namespace_handler.h>
#include <lyric_compiler/root_handler.h>
#include <lyric_compiler/typename_handler.h>
#include <lyric_compiler/using_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::RootHandler::RootHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::RootHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    if (!node->isClass(lyric_schema::kLyricAstRootClass))
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid node for root");

    TU_LOG_VV << "before RootHandler@" << this;

    auto *driver = getDriver();

    auto *root = driver->getObjectRoot();
    auto *globalNamespace = root->globalNamespace();
    auto *rootBlock = root->rootBlock();

    auto numChildren = node->numChildren();

    for (int i = 0; i < numChildren; i++) {
        auto *child = node->getChild(i);

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));

        // definitions in the root are placed in the global namespace
        switch (astId) {
            case lyric_schema::LyricAstId::Namespace: {
                auto ns = std::make_unique<NamespaceHandler>(
                    globalNamespace, /* isSideEffect= */ true, rootBlock, driver);
                ctx.appendGrouping(std::move(ns));
                break;
            }
            case lyric_schema::LyricAstId::Alias: {
                auto handler = std::make_unique<AliasHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::Def: {
                auto handler = std::make_unique<DefHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefClass: {
                auto handler = std::make_unique<DefClassHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefConcept: {
                auto handler = std::make_unique<DefConceptHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefEnum: {
                auto handler = std::make_unique<DefEnumHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefInstance: {
                auto handler = std::make_unique<DefInstanceHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefStatic: {
                auto handler = std::make_unique<DefStaticHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefStruct: {
                auto handler = std::make_unique<DefStructHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::TypeName: {
                auto handler = std::make_unique<TypenameHandler>(
                    /* isSideEffect= */ true, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendChoice(std::move(handler));
                break;
            }
             case lyric_schema::LyricAstId::Using: {
                 auto handler = std::make_unique<RootUsingHandler>(driver);
                 ctx.appendGrouping(std::move(handler));
                 break;
             }
            case lyric_schema::LyricAstId::ImportAll:
            case lyric_schema::LyricAstId::ImportModule:
            case lyric_schema::LyricAstId::ImportSymbols: {
                auto handler = std::make_unique<ImportHandler>(
                    globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            default: {
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "invalid node for root");
            }
        }
    }

    // if root node has an entry offset attr then we expect the last child to be the entry node
    if (node->hasAttr(lyric_parser::kLyricAstEntryOffset)) {
        auto entry = std::make_unique<EntryHandler>(driver);
        ctx.appendGrouping(std::move(entry));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::RootHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after RootHandler@" << this;
    return {};
}