#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/static_loader.h>
#include <lyric_runtime/operand.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/memory_bytes.h>

#include "runtime_mocks.h"

class PointerOperand : public ::testing::Test {
protected:
    lyric_common::ModuleLocation testmodLocation;
    lyric_object::LyricObject testmodObject;
    std::shared_ptr<lyric_runtime::StaticLoader> staticLoader;
    std::shared_ptr<lyric_runtime::InterpreterState> state;

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
    }
};


TEST_F (PointerOperand, RoundtripAbstractRef)
{
    MockRef ref;
    lyric_runtime::AbstractRef *in = &ref;
    auto value = lyric_runtime::Operand::fromRef(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Ref, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::AbstractRef *out;
    ASSERT_TRUE (value.getRef(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, ConversionFromAbstractRefFailsWhenNullPointer)
{
    lyric_runtime::AbstractRef *in = nullptr;
    auto value = lyric_runtime::Operand::fromRef(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::DataCellType::Invalid, value.getType());
}

TEST_F (PointerOperand, RoundtripBytes)
{
    auto *heapManager = state->heapManager();
    auto bytes = tempo_utils::MemoryBytes::copy("hello, world!");
    auto cell = heapManager->allocateBytes(bytes->getSpan());

    lyric_runtime::BytesRef *in = cell.data.bytes;
    auto value = lyric_runtime::Operand::fromBytes(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Bytes, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::BytesRef *out;
    ASSERT_TRUE (value.getBytes(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, RoundtripStatus)
{
    auto *heapManager = state->heapManager();
    auto cell = heapManager->allocateStatus(tempo_utils::StatusCode::kInternal, "failed");

    lyric_runtime::StatusRef *in = cell.data.status;
    auto value = lyric_runtime::Operand::fromStatus(in);
    ASSERT_EQ (lyric_runtime::DataCellType::Status, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::StatusRef *out;
    ASSERT_TRUE (value.getStatus(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, RoundtripString)
{
    auto *heapManager = state->heapManager();
    auto cell = heapManager->allocateString("hello, world!");

    lyric_runtime::StringRef *in = cell.data.str;
    auto value = lyric_runtime::Operand::fromString(in);
    ASSERT_EQ (lyric_runtime::DataCellType::String, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::StringRef *out;
    ASSERT_TRUE (value.getString(out));
    ASSERT_EQ (in, out);
}
