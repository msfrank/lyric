#include <gtest/gtest.h>

#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_importer/module_cache.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/scope_manager.h>
#include <tempo_tracing/trace_recorder.h>

#include "compiler_mocks.h"

TEST(CompilerScanDriver, InitializeDriver)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RAISE (objectRoot, objectState.defineRoot());

    lyric_compiler::CompilerScanDriver driver(objectRoot, &objectState);

    auto rootHandler = std::make_unique<MockGrouping>(&driver);
    ASSERT_THAT (driver.initialize(std::move(rootHandler)), tempo_test::IsOk());
}

TEST(CompilerScanDriver, HandleRootNode)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RAISE (objectRoot, objectState.defineRoot());

    lyric_parser::ArchetypeState archetypeState(location.toUrl(), &scopeManager);
    lyric_parser::ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, archetypeState.appendNode(lyric_schema::kLyricAstBlockClass, {}));
    archetypeState.setRoot(blockNode);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RAISE (archetype, archetypeState.toArchetype());

    auto driver = std::make_shared<lyric_compiler::CompilerScanDriver>(objectRoot, &objectState);

    auto rootHandler = std::make_unique<MockGrouping>(driver.get());
    EXPECT_CALL (*rootHandler, before)
        .Times(1)
        .WillOnce(::testing::Return(tempo_utils::Status{}));
    EXPECT_CALL (*rootHandler, after)
        .Times(1)
        .WillOnce(::testing::Return(tempo_utils::Status{}));

    ASSERT_THAT (driver->initialize(std::move(rootHandler)), tempo_test::IsOk());

    lyric_rewriter::RewriterOptions options;
    lyric_rewriter::LyricRewriter rewriter(options);
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), driver, recorder), tempo_test::IsOk());
}

TEST(CompilerScanDriver, HandleRootAndSingleChild)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RAISE (objectRoot, objectState.defineRoot());

    lyric_parser::ArchetypeState archetypeState(location.toUrl(), &scopeManager);
    lyric_parser::ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, archetypeState.appendNode(lyric_schema::kLyricAstBlockClass, {}));
    archetypeState.setRoot(blockNode);

    lyric_parser::ArchetypeNode *child1;
    TU_ASSIGN_OR_RAISE (child1, archetypeState.appendNode(lyric_schema::kLyricAstNilClass, {}));
    blockNode->appendChild(child1);

    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RAISE (archetype, archetypeState.toArchetype());

    auto driver = std::make_shared<lyric_compiler::CompilerScanDriver>(objectRoot, &objectState);

    auto rootHandler = std::make_unique<MockGrouping>(driver.get());
    EXPECT_CALL (*rootHandler, before)
        .Times(1)
        .WillOnce([](
            const lyric_parser::ArchetypeState *,
            const lyric_parser::ArchetypeNode *,
            lyric_compiler::BeforeContext &ctx) -> tempo_utils::Status {
            auto behavior = std::make_unique<MockBehavior>();
            EXPECT_CALL (*behavior, enter)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            EXPECT_CALL (*behavior, exit)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            ctx.appendBehavior(std::move(behavior));
            return {};
        });
    EXPECT_CALL (*rootHandler, after)
        .Times(1)
        .WillOnce(::testing::Return(tempo_utils::Status{}));

    ASSERT_THAT (driver->initialize(std::move(rootHandler)), tempo_test::IsOk());

    lyric_rewriter::RewriterOptions options;
    lyric_rewriter::LyricRewriter rewriter(options);
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), driver, recorder), tempo_test::IsOk());
}

TEST(CompilerScanDriver, HandleRootAndMultipleChildren)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    auto systemModuleCache = lyric_importer::ModuleCache::create(loader);
    auto recorder = tempo_tracing::TraceRecorder::create();
    tempo_tracing::ScopeManager scopeManager(recorder);
    lyric_assembler::ObjectState objectState(location, systemModuleCache, &scopeManager);

    lyric_assembler::ObjectRoot *objectRoot;
    TU_ASSIGN_OR_RAISE (objectRoot, objectState.defineRoot());

    lyric_parser::ArchetypeState archetypeState(location.toUrl(), &scopeManager);
    lyric_parser::ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, archetypeState.appendNode(lyric_schema::kLyricAstBlockClass, {}));
    archetypeState.setRoot(blockNode);

    lyric_parser::ArchetypeNode *child1;
    TU_ASSIGN_OR_RAISE (child1, archetypeState.appendNode(lyric_schema::kLyricAstNilClass, {}));
    blockNode->appendChild(child1);

    lyric_parser::ArchetypeNode *child2;
    TU_ASSIGN_OR_RAISE (child2, archetypeState.appendNode(lyric_schema::kLyricAstTrueClass, {}));
    blockNode->appendChild(child2);

    lyric_parser::ArchetypeNode *child3;
    TU_ASSIGN_OR_RAISE (child3, archetypeState.appendNode(lyric_schema::kLyricAstFalseClass, {}));
    blockNode->appendChild(child3);

    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RAISE (archetype, archetypeState.toArchetype());

    auto driver = std::make_shared<lyric_compiler::CompilerScanDriver>(objectRoot, &objectState);

    auto rootHandler = std::make_unique<MockGrouping>(driver.get());
    EXPECT_CALL (*rootHandler, before)
        .Times(1)
        .WillOnce([](
            const lyric_parser::ArchetypeState *,
            const lyric_parser::ArchetypeNode *,
            lyric_compiler::BeforeContext &ctx) -> tempo_utils::Status
        {
            auto behavior1 = std::make_unique<MockBehavior>();
            EXPECT_CALL (*behavior1, enter)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            EXPECT_CALL (*behavior1, exit)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            ctx.appendBehavior(std::move(behavior1));
            auto behavior2 = std::make_unique<MockBehavior>();
            EXPECT_CALL (*behavior2, enter)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            EXPECT_CALL (*behavior2, exit)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            ctx.appendBehavior(std::move(behavior2));
            auto behavior3 = std::make_unique<MockBehavior>();
            EXPECT_CALL (*behavior3, enter)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            EXPECT_CALL (*behavior3, exit)
                .Times(1)
                .WillOnce(::testing::Return(tempo_utils::Status{}));
            ctx.appendBehavior(std::move(behavior3));
            return {};
        });
    EXPECT_CALL (*rootHandler, after)
        .Times(1)
        .WillOnce(::testing::Return(tempo_utils::Status{}));

    ASSERT_THAT (driver->initialize(std::move(rootHandler)), tempo_test::IsOk());

    lyric_rewriter::RewriterOptions options;
    lyric_rewriter::LyricRewriter rewriter(options);
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), driver, recorder), tempo_test::IsOk());
}
