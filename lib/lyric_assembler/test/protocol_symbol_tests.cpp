#include <gtest/gtest.h>

#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_runtime/static_loader.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_tracing/trace_recorder.h>
#include <tempo_utils/uuid.h>

#include "base_assembler_fixture.h"
#include "lyric_assembler/fundamental_cache.h"
#include "lyric_assembler/protocol_symbol.h"

class ProtocolSymbol : public BaseAssemblerFixture {};

TEST_F (ProtocolSymbol, DeclareProtocol)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *block = objectRoot->rootBlock();

    auto BytesType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bytes);

    auto declareProtocolResult = block->declareProtocol("test", false, BytesType, BytesType,
        lyric_object::PortType::Connect, lyric_object::CommunicationType::SendAndReceive);
    ASSERT_THAT (declareProtocolResult, tempo_test::IsResult());
    auto *protocolSymbol = declareProtocolResult.getResult();

    ASSERT_EQ ("test", protocolSymbol->getSymbolUrl().getSymbolName());
    ASSERT_FALSE (protocolSymbol->isHidden());
    ASSERT_FALSE (protocolSymbol->isDeclOnly());
    ASSERT_EQ (BytesType, protocolSymbol->sendType()->getTypeDef());
    ASSERT_EQ (BytesType, protocolSymbol->receiveType()->getTypeDef());
    ASSERT_EQ (lyric_object::PortType::Connect, protocolSymbol->getPortType());
    ASSERT_EQ (lyric_object::CommunicationType::SendAndReceive, protocolSymbol->getCommunicationType());
}

TEST_F (ProtocolSymbol, WriteProtocol)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *globalNs = objectRoot->globalNamespace();
    auto *block = globalNs->namespaceBlock();

    auto BytesType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bytes);

    auto declareProtocolResult = block->declareProtocol("test", false, BytesType, BytesType,
        lyric_object::PortType::Connect, lyric_object::CommunicationType::SendAndReceive);
    ASSERT_THAT (declareProtocolResult, tempo_test::IsResult());
    auto *protocolSymbol = declareProtocolResult.getResult();

    TU_RAISE_IF_NOT_OK (globalNs->putTarget(protocolSymbol->getSymbolUrl()));

    lyric_object::LyricObject object;
    ASSERT_THAT (writeObjectWithEmptyEntry(object), tempo_test::IsOk());

    ASSERT_EQ (1, object.numProtocols());
    auto walker = object.getProtocol(0);
    ASSERT_TRUE (walker.isValid());
    ASSERT_EQ (protocolSymbol->getSymbolUrl().getSymbolPath(), walker.getSymbolPath());
    ASSERT_FALSE (walker.isDeclOnly());
    ASSERT_EQ (lyric_object::AccessType::Public, walker.getAccess());
    ASSERT_EQ (lyric_object::PortType::Connect, walker.getPort());
    ASSERT_EQ (lyric_object::CommunicationType::SendAndReceive, walker.getCommunication());
    ASSERT_EQ (BytesType, walker.getSendType().getTypeDef());
    ASSERT_EQ (BytesType, walker.getReceiveType().getTypeDef());
}
