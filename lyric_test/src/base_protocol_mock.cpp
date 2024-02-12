
#include <lyric_test/base_protocol_mock.h>
#include <lyric_test/test_result.h>

lyric_test::BaseProtocolMock::BaseProtocolMock()
    : m_loop(nullptr)
{
}

bool
lyric_test::BaseProtocolMock::isAttached()
{
    return m_port != nullptr;
}

tempo_utils::Status
lyric_test::BaseProtocolMock::attach(uv_loop_t *loop, std::shared_ptr<lyric_runtime::DuplexPort> port)
{
    m_loop = loop;
    m_port = port;
    m_port->attach(this);
    TU_LOG_INFO << "attached BaseProtocolMock";
    Receive receive;
    receive.type = ReceiveType::ATTACH;
    return handle(receive);
}

tempo_utils::Status
lyric_test::BaseProtocolMock::detach()
{
    m_port->detach();
    m_port.reset();
    m_loop = nullptr;
    TU_LOG_INFO << "detached BaseProtocolMock";
    Receive receive;
    receive.type = ReceiveType::DETACH;
    return handle(receive);
}

tempo_utils::Status
lyric_test::BaseProtocolMock::send(const lyric_serde::LyricPatchset &patchset)
{
    if (m_port == nullptr)
        return TestStatus::forCondition(TestCondition::kTestInvariant, "port is detached");
    m_port->receive(patchset);
    return TestStatus::ok();
}

tempo_utils::Status
lyric_test::BaseProtocolMock::write(const lyric_serde::LyricPatchset &patchset)
{
    Receive receive;
    receive.type = ReceiveType::PATCHSET;
    receive.patchset = patchset;
    auto status = handle(receive);
    if (status.notOk())
        return status;
    return TestStatus::ok();
}