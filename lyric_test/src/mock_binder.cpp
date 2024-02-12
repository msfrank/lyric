
#include <lyric_test/mock_binder.h>

lyric_test::MockBinder::MockBinder(
    const absl::flat_hash_map<tempo_utils::Url, std::shared_ptr<BaseProtocolMock>> &protocolMocks)
    : m_protocolMocks(protocolMocks)
{
}

tempo_utils::Result<lyric_runtime::Return>
lyric_test::MockBinder::run(lyric_runtime::BytecodeInterpreter *interp)
{
    auto *state = interp->interpreterState();
    auto *loop = state->mainLoop();
    auto *multiplexer = state->portMultiplexer();

    // attach each mock to its port
    for (const auto &protocolMock : m_protocolMocks) {
        auto protocolUrl = protocolMock.first;
        auto mockHandler = protocolMock.second;
        auto registerPortResult = multiplexer->registerPort(protocolUrl);
        if (registerPortResult.isStatus())
            return registerPortResult.getStatus();
        auto port = registerPortResult.getResult();
        mockHandler->attach(loop, port);
    }

    // run the interpreter until termination
    auto runInterpreterResult = interp->run();

    // detach mocks

    // return interpreter execution result
    if (runInterpreterResult.isStatus())
        return runInterpreterResult.getStatus();
    return runInterpreterResult.getResult();
}
