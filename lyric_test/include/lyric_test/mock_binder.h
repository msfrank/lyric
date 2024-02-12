#ifndef LYRIC_TEST_MOCK_BINDER_H
#define LYRIC_TEST_MOCK_BINDER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_test/base_protocol_mock.h>
#include <lyric_test/test_result.h>
#include <tempo_utils/url.h>

namespace lyric_test {

    class MockBinder {

    public:
        MockBinder(
            const absl::flat_hash_map<
                tempo_utils::Url,
                std::shared_ptr<BaseProtocolMock>> &protocolMocks);

        tempo_utils::Result<lyric_runtime::Return> run(lyric_runtime::BytecodeInterpreter *interp);

    private:
        absl::flat_hash_map<tempo_utils::Url, std::shared_ptr<BaseProtocolMock>> m_protocolMocks;
    };
}

#endif // LYRIC_TEST_MOCK_BINDER_H