#include <gtest/gtest.h>

#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_compiler/base_grouping.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_importer/module_cache.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/trace_recorder.h>

#include "compiler_mocks.h"

class CompilerScanDriver : public ::testing::Test {
protected:
    lyric_common::ModuleLocation location;
    lyric_common::ModuleLocation origin;
    std::shared_ptr<lyric_runtime::StaticLoader> staticLoader;
    std::shared_ptr<lyric_bootstrap::BootstrapLoader> bootstrapLoader;
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache;
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache;
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver;
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder;
    std::unique_ptr<lyric_assembler::ObjectState> objectState;
    lyric_assembler::ObjectRoot *objectRoot;

    void SetUp() override {
        location = lyric_common::ModuleLocation::fromString("/test");
        origin = lyric_common::ModuleLocation::fromString("test://");
        staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
        bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
        localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
        systemModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
        shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
        recorder = tempo_tracing::TraceRecorder::create();
        objectState = std::make_unique<lyric_assembler::ObjectState>(
            location, origin, localModuleCache, systemModuleCache, shortcutResolver);
        TU_ASSIGN_OR_RAISE (objectRoot, objectState->defineRoot());
    }
};

class TestCompilerScanDriverBuilder : public lyric_rewriter::AbstractScanDriverBuilder {
public:
    std::shared_ptr<lyric_compiler::CompilerScanDriver> driver;

    TestCompilerScanDriverBuilder(lyric_assembler::ObjectRoot *root, lyric_assembler::ObjectState *state) {
        driver = std::make_shared<lyric_compiler::CompilerScanDriver>(root, state);
    }
    tempo_utils::Status applyPragma(const lyric_parser::ArchetypeState *state, const lyric_parser::ArchetypeNode *node) override {
        return {};
    }
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>> makeScanDriver() override {
        return std::static_pointer_cast<lyric_rewriter::AbstractScanDriver>(driver);
    }
};

TEST_F(CompilerScanDriver, InitializeDriver)
{
    lyric_compiler::CompilerScanDriver driver(objectRoot, objectState.get());
    auto rootHandler = std::make_unique<MockGrouping>(&driver);
    ASSERT_THAT (driver.initialize(std::move(rootHandler)), tempo_test::IsOk());
}

TEST_F(CompilerScanDriver, HandleRootNode)
{
    lyric_parser::ArchetypeState archetypeState(location.toUrl());
    lyric_parser::ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, archetypeState.appendNode(lyric_schema::kLyricAstBlockClass, {}));
    archetypeState.setRoot(blockNode);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RAISE (archetype, archetypeState.toArchetype());

    auto builder = std::make_shared<TestCompilerScanDriverBuilder>(objectRoot, objectState.get());
    auto driver = builder->driver;

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
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), builder, recorder), tempo_test::IsOk());
}

TEST_F(CompilerScanDriver, HandleRootAndSingleChild)
{
    lyric_parser::ArchetypeState archetypeState(location.toUrl());
    lyric_parser::ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, archetypeState.appendNode(lyric_schema::kLyricAstBlockClass, {}));
    archetypeState.setRoot(blockNode);

    lyric_parser::ArchetypeNode *child1;
    TU_ASSIGN_OR_RAISE (child1, archetypeState.appendNode(lyric_schema::kLyricAstNilClass, {}));
    blockNode->appendChild(child1);

    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RAISE (archetype, archetypeState.toArchetype());

    auto builder = std::make_shared<TestCompilerScanDriverBuilder>(objectRoot, objectState.get());
    auto driver = builder->driver;

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
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), builder, recorder), tempo_test::IsOk());
}

TEST_F(CompilerScanDriver, HandleRootAndMultipleChildren)
{
    lyric_parser::ArchetypeState archetypeState(location.toUrl());
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

    auto builder = std::make_shared<TestCompilerScanDriverBuilder>(objectRoot, objectState.get());
    auto driver = builder->driver;

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
    ASSERT_THAT (rewriter.scanArchetype(archetype, location.toUrl(), builder, recorder), tempo_test::IsOk());
}
