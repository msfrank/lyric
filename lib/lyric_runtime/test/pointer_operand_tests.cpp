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

class BaseRefImpl : public lyric_runtime::BaseRef {
public:
    BaseRefImpl(): BaseRef(nullptr) {};
    ~BaseRefImpl() override = default;
    std::string toString() const override { return {}; }

protected:
    void setMembersReachable() override {};
    void clearMembersReachable() override {};
};

TEST_F (PointerOperand, RoundtripBaseRef)
{
    BaseRefImpl ref;
    lyric_runtime::BaseRef *in = &ref;
    auto value = lyric_runtime::Operand::fromRef(in);
    ASSERT_EQ (lyric_runtime::OperandType::Ref, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::BaseRef *out;
    ASSERT_TRUE (value.getRef(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, ConversionFromBaseRefFailsWhenNullPointer)
{
    lyric_runtime::BaseRef *in = nullptr;
    auto value = lyric_runtime::Operand::fromRef(in);
    ASSERT_FALSE (value.isValid());
    ASSERT_EQ (lyric_runtime::OperandType::Invalid, value.getType());
}

TEST_F (PointerOperand, RoundtripBytes)
{
    auto *heapManager = state->heapManager();
    auto bytes = tempo_utils::MemoryBytes::copy("hello, world!");
    auto cell = heapManager->allocateBytes(bytes->getSpan());

    lyric_runtime::BytesRef *in;
    ASSERT_TRUE (cell.getBytes(in));
    auto value = lyric_runtime::Operand::fromBytes(in);
    ASSERT_EQ (lyric_runtime::OperandType::Bytes, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::BytesRef *out;
    ASSERT_TRUE (value.getBytes(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, RoundtripStatus)
{
    auto *heapManager = state->heapManager();
    auto cell = heapManager->allocateStatus(tempo_utils::StatusCode::kInternal, "failed");

    lyric_runtime::StatusRef *in;
    ASSERT_TRUE (cell.getStatus(in));
    auto value = lyric_runtime::Operand::fromStatus(in);
    ASSERT_EQ (lyric_runtime::OperandType::Status, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::StatusRef *out;
    ASSERT_TRUE (value.getStatus(out));
    ASSERT_EQ (in, out);
}

TEST_F (PointerOperand, RoundtripString)
{
    auto *heapManager = state->heapManager();
    auto cell = heapManager->allocateString("hello, world!", false);

    lyric_runtime::StringRef *in;
    ASSERT_TRUE (cell.getString(in));
    auto value = lyric_runtime::Operand::fromString(in);
    ASSERT_EQ (lyric_runtime::OperandType::String, value.getType());
    ASSERT_EQ (lyric_runtime::OverlayType::Pointer, value.getOverlay());
    lyric_runtime::StringRef *out;
    ASSERT_TRUE (value.getString(out));
    ASSERT_EQ (in, out);
}
