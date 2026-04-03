
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_runtime_fixture.h"

class SystemScheduler : public BaseRuntimeFixture {
protected:
    std::shared_ptr<lyric_runtime::InterpreterState> state;

    void SetUp() override {
        BaseRuntimeFixture::SetUp();
        auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
        TU_ASSIGN_OR_RAISE (state, lyric_runtime::InterpreterState::create(loader, loader));
    }

    void TearDown() override {
        BaseRuntimeFixture::TearDown();
    }
};

TEST_F (SystemScheduler, MainTaskIsCreatedDuringInitialization)
{
    auto *systemScheduler = state->systemScheduler();

    auto *task = systemScheduler->mainTask();
    ASSERT_TRUE (task != nullptr);
    ASSERT_TRUE (task->isMainTask());
    ASSERT_EQ (lyric_runtime::Task::State::Waiting, task->getState());
    ASSERT_EQ (task, systemScheduler->firstWaitingTask());

    auto *ready = systemScheduler->firstReadyTask();
    ASSERT_TRUE (ready == nullptr);

    auto *current = systemScheduler->currentTask();
    ASSERT_TRUE (current == nullptr);
}

TEST_F (SystemScheduler, SelectNextReady)
{
    auto *systemScheduler = state->systemScheduler();

    auto *task = systemScheduler->mainTask();
    systemScheduler->resumeTask(task);
    ASSERT_EQ (lyric_runtime::Task::State::Ready, task->getState());

    auto *ready = systemScheduler->firstReadyTask();
    ASSERT_EQ (task, ready);
    auto *current = systemScheduler->currentTask();
    ASSERT_TRUE (current == nullptr);
}

TEST_F (SystemScheduler, SelectNextReadyReturnsNullptrWhenNoReadyTasks)
{
    auto *systemScheduler = state->systemScheduler();

    auto *next = systemScheduler->selectNextReady();
    ASSERT_TRUE (next == nullptr);

    auto *ready = systemScheduler->firstReadyTask();
    ASSERT_TRUE (ready == nullptr);

    auto *current = systemScheduler->currentTask();
    ASSERT_TRUE (current == nullptr);
}

void on_timer(uv_timer_t *timer)
{
    auto *async = static_cast<lyric_runtime::AsyncHandle *>(timer->data);
    async->sendSignal();
}

void on_accept(lyric_runtime::Promise *promise, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state)
{
    promise->complete(lyric_runtime::DataCell(true));
    uv_stop(state->mainLoop());
}

TEST_F (SystemScheduler, RegisterAsync)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = systemScheduler->systemLoop();

    auto promise = lyric_runtime::Promise::create(on_accept);

    auto registerAsyncResult = systemScheduler->registerAsync(promise);
    ASSERT_THAT (registerAsyncResult, tempo_test::IsResult());
    auto async = registerAsyncResult.getResult();
    ASSERT_TRUE (async->isPending());

    uv_timer_t timer;
    uv_timer_init(loop, &timer);
    timer.data = async.get();
    uv_timer_start(&timer, on_timer, 100, 0);

    uv_run(loop, UV_RUN_DEFAULT);

    ASSERT_FALSE (async->isPending());
    ASSERT_EQ (lyric_runtime::Promise::State::Completed, promise->getState());
    auto result = promise->getResult();
    ASSERT_THAT (result, DataCellBool(true));
}

TEST_F (SystemScheduler, RegisterTimer)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = systemScheduler->systemLoop();

    auto promise = lyric_runtime::Promise::create(on_accept);

    ASSERT_THAT (systemScheduler->registerTimer(100, promise), tempo_test::IsOk());

    auto *waiter = systemScheduler->firstWaiter();
    ASSERT_TRUE (waiter != nullptr);
    ASSERT_EQ (lyric_runtime::Waiter::Type::Handle, waiter->getType());
    ASSERT_TRUE (waiter->hasPromise());

    uv_run(loop, UV_RUN_DEFAULT);

    ASSERT_EQ (lyric_runtime::Promise::State::Completed, promise->getState());
    auto result = promise->getResult();
    ASSERT_THAT (result, DataCellBool(true));
}
