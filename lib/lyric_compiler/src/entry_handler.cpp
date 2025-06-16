
#include <lyric_assembler/object_root.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/defalias_handler.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/defconcept_handler.h>
#include <lyric_compiler/defenum_handler.h>
#include <lyric_compiler/definstance_handler.h>
#include <lyric_compiler/defstatic_handler.h>
#include <lyric_compiler/defstruct_handler.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_compiler/import_handler.h>
#include <lyric_compiler/namespace_handler.h>
#include <lyric_compiler/typename_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::EntryHandler::EntryHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::EntryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    if (!node->isClass(lyric_schema::kLyricAstBlockClass))
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid node for entry");

    TU_LOG_INFO << "before EntryHandler@" << this;

    auto *driver = getDriver();

    auto *root = driver->getObjectRoot();
    auto *entryCall = root->entryCall();
    auto *globalNamespace = root->globalNamespace();
    auto *rootBlock = root->rootBlock();

    auto *entryProc = entryCall->callProc();
    auto *entryBlock = entryProc->procBlock();
    auto *entryCode = entryProc->procCode();
    auto *fragment = entryCode->rootFragment();

    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    for (int i = 0; i < numChildren; i++) {
        auto *child = node->getChild(i);

        bool isSideEffect = i < numChildren - 1;
        auto formType = isSideEffect? FormType::SideEffect : FormType::Any;

        // delegate any nodes not in AST namespace to the form handler
        if (!child->isNamespace(lyric_schema::kLyricAstNs)) {
            auto any = std::make_unique<FormChoice>(formType, fragment, entryBlock, driver);
            ctx.appendChoice(std::move(any));
            continue;
        }

        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(child->getIdValue());

        auto astId = resource->getId();
        switch (astId) {
            case lyric_schema::LyricAstId::Namespace: {
                auto ns = std::make_unique<NamespaceHandler>(
                    globalNamespace, isSideEffect, rootBlock, driver);
                ctx.appendGrouping(std::move(ns));
                break;
            }
            case lyric_schema::LyricAstId::Def: {
                auto handler = std::make_unique<DefHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefAlias: {
                auto handler = std::make_unique<DefAliasHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefClass: {
                auto handler = std::make_unique<DefClassHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefConcept: {
                auto handler = std::make_unique<DefConceptHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefEnum: {
                auto handler = std::make_unique<DefEnumHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefInstance: {
                auto handler = std::make_unique<DefInstanceHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefStatic: {
                auto handler = std::make_unique<DefStaticHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefStruct: {
                auto handler = std::make_unique<DefStructHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::TypeName: {
                auto handler = std::make_unique<TypenameHandler>(
                    isSideEffect, globalNamespace, globalNamespace->namespaceBlock(), driver);
                ctx.appendChoice(std::move(handler));
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
                auto any = std::make_unique<FormChoice>(formType, fragment, entryBlock, driver);
                ctx.appendChoice(std::move(any));
                break;
            }
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::EntryHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after EntryHandler@" << this;
    return {};
}