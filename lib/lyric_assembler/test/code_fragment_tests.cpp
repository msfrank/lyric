#include <gtest/gtest.h>

#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_runtime/static_loader.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_utils/uuid.h>

TEST(CodeFragment, ImmediateNil)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto systemModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    auto recorder = tempo_tracing::TraceRecorder::create();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_assembler::ObjectState objectState(
        location, origin, localModuleCache, systemModuleCache, shortcutResolver);

    lyric_assembler::ObjectRoot *root;
    TU_ASSIGN_OR_RAISE (root, objectState.defineRoot());

    auto activationUrl = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle procHandle(activationUrl, root->rootBlock(), &objectState);
    auto *fragment = procHandle.procFragment();

    ASSERT_THAT (fragment->immediateNil(), tempo_test::IsOk());
    ASSERT_THAT (fragment->returnToCaller(), tempo_test::IsOk());

    lyric_assembler::ObjectWriter objectWriter(&objectState);
    lyric_object::BytecodeBuilder bytecodeBuilder;
    tempo_utils::BytesAppender bytesAppender;
    ASSERT_THAT (procHandle.build(objectWriter, bytesAppender), tempo_test::IsOk());
    auto bytecode = bytesAppender.finish();

    lyric_object::ProcInfo procInfo;
    ASSERT_THAT (lyric_object::parse_proc_info(bytecode->getSpan(), 0, procInfo), tempo_test::IsOk());
    lyric_object::BytecodeIterator it(procInfo.code);
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NIL, op.opcode);
    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_RETURN, op.opcode);
    ASSERT_FALSE (it.hasNext());
}

TEST(CodeFragment, UnconditionalJump)
{
    auto location = lyric_common::ModuleLocation::fromString("/test");
    auto staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
    auto bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    auto localModuleCache = lyric_importer::ModuleCache::create(staticLoader);
    auto systemModuleCache = lyric_importer::ModuleCache::create(bootstrapLoader);
    auto shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    auto recorder = tempo_tracing::TraceRecorder::create();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("tester://", tempo_utils::UUID::randomUUID().toString()));

    lyric_assembler::ObjectState objectState(
        location, origin, localModuleCache, systemModuleCache, shortcutResolver);

    lyric_assembler::ObjectRoot *root;
    TU_ASSIGN_OR_RAISE (root, objectState.defineRoot());

    auto activationUrl = lyric_common::SymbolUrl::fromString("#sym");
    lyric_assembler::ProcHandle procHandle(activationUrl, root->rootBlock(), &objectState);
    auto *fragment = procHandle.procFragment();

    lyric_assembler::JumpLabel label;
    TU_ASSIGN_OR_RAISE (label, fragment->appendLabel("top"));

    ASSERT_THAT (fragment->noOperation(), tempo_test::IsOk());
    lyric_assembler::JumpTarget target;
    TU_ASSIGN_OR_RAISE (target, fragment->unconditionalJump());
    ASSERT_THAT (fragment->patchTarget(target, label), tempo_test::IsOk());
    ASSERT_THAT (fragment->returnToCaller(), tempo_test::IsOk());

    lyric_assembler::ObjectWriter objectWriter(&objectState);
    tempo_utils::BytesAppender bytesAppender;
    ASSERT_THAT (procHandle.build(objectWriter, bytesAppender), tempo_test::IsOk());
    auto bytecode = bytesAppender.finish();

    lyric_object::ProcInfo procInfo;
    ASSERT_THAT (lyric_object::parse_proc_info(bytecode->getSpan(), 0, procInfo), tempo_test::IsOk());
    lyric_object::BytecodeIterator it(procInfo.code);
    lyric_object::OpCell op;

    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_NOOP, op.opcode);
    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_JUMP, op.opcode);
    ASSERT_EQ (lyric_object::OpInfoType::JUMP_I16, op.type);
    ASSERT_EQ (-4, op.operands.jump_i16.jump);
    ASSERT_TRUE (it.getNext(op));
    ASSERT_EQ (lyric_object::Opcode::OP_RETURN, op.opcode);

    ASSERT_FALSE (it.hasNext());
}
