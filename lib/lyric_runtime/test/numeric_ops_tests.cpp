#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_runtime/internal/numeric_ops.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/operand.h>
#include <lyric_runtime/static_loader.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/memory_bytes.h>

class NumericOps : public ::testing::Test {
protected:
    lyric_common::ModuleLocation testmodLocation;
    lyric_object::LyricObject testmodObject;
    std::shared_ptr<lyric_runtime::StaticLoader> staticLoader;
    std::shared_ptr<lyric_runtime::InterpreterState> state;
    lyric_runtime::HeapManager *heapManager;

    void SetUp() override {
        staticLoader = std::make_shared<lyric_runtime::StaticLoader>();
        testmodLocation = lyric_common::ModuleLocation::fromString("test:///testmod");
        tempo_utils::FileReader reader(TESTMOD_OBJECT_PATH);
        TU_RAISE_IF_NOT_OK (reader.getStatus());
        testmodObject = lyric_object::LyricObject(reader.getBytes());
        staticLoader->insertModule(testmodLocation, testmodObject);
        auto systemLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
        TU_ASSIGN_OR_RAISE (state, lyric_runtime::InterpreterState::create(systemLoader, staticLoader));
        TU_RAISE_IF_NOT_OK (state->load(testmodLocation));
        heapManager = state->heapManager();
    }
};

TEST_F (NumericOps, Add)
{
    auto lhs = lyric_runtime::Operand::fromI32(101);
    auto rhs = lyric_runtime::Operand::fromI32(202);
    lyric_runtime::Operand result;
    ASSERT_THAT (lyric_runtime::internal::add(heapManager, lhs, rhs, result), tempo_test::IsOk());
    tu_int32 i32;
    ASSERT_TRUE (result.getI32(i32));
    ASSERT_EQ (303, i32);
}