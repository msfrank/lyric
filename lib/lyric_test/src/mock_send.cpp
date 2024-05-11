
#include <lyric_test/mock_send.h>
#include <lyric_test/test_result.h>

lyric_test::MockSend::MockSend()
{
}

tempo_utils::Status
lyric_test::MockSend::handle(Receive receive)
{
    switch (receive.type) {
        case ReceiveType::ATTACH:
        case ReceiveType::DETACH:
            return TestStatus::ok();
        case ReceiveType::PATCHSET:
            m_messages.push_back(receive.patchset);
            return TestStatus::ok();
        default:
            return TestStatus::forCondition(TestCondition::kTestInvariant, "unexpected receive type");
    }
}

std::vector<lyric_serde::LyricPatchset>
lyric_test::MockSend::getMessages() const
{
    return m_messages;
}
