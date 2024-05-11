
#include <lyric_test/mock_receive.h>
#include <lyric_test/test_result.h>

lyric_test::MockReceive::MockReceive(const std::vector<lyric_serde::LyricPatchset> &messages)
    : m_messages(messages)
{
}

tempo_utils::Status
lyric_test::MockReceive::handle(Receive receive)
{
    if (receive.type != ReceiveType::ATTACH)
        return TestStatus::forCondition(TestCondition::kTestInvariant, "expected attach");

    for (const auto &message : m_messages) {
        auto status = send(message);
        if (status.notOk())
            return status;
    }

    return TestStatus::ok();
}
