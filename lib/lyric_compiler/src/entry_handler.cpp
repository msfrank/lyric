
#include <lyric_assembler/object_root.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_compiler/namespace_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::EntryHandler::EntryHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::EntryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
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

        if (!child->isNamespace(lyric_schema::kLyricAstNs))
            return {};
        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(child->getIdValue());

        auto astId = resource->getId();
        switch (astId) {
            case lyric_schema::LyricAstId::Namespace: {
                auto ns = std::make_unique<NamespaceHandler>(
                    globalNamespace, true, rootBlock, driver);
                ctx.appendGrouping(std::move(ns));
                break;
            }
            case lyric_schema::LyricAstId::Def: {
                auto handler = std::make_unique<DefHandler>(
                    /* isSideEffect= */ true, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            case lyric_schema::LyricAstId::DefClass: {
                auto handler = std::make_unique<DefClassHandler>(
                    /* isSideEffect= */ true, globalNamespace->namespaceBlock(), driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            default: {
                auto any = std::make_unique<FormChoice>(
                    FormType::Any, fragment, entryBlock, driver);
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
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after EntryHandler@" << this;
    return {};
}