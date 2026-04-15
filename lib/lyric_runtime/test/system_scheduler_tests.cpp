
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

class AllOfOps : public lyric_runtime::PromiseOperations {
public:
    void onPartial(
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsBegin,
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsEnd,
        std::shared_ptr<lyric_runtime::AsyncHandle> &done) override {
        int numdeps = 0;
        int numdone = 0;
        for (; depsBegin != depsEnd; ++depsBegin) {
            numdeps++;
            switch ((*depsBegin)->getState()) {
                case lyric_runtime::Promise::State::Completed:
                case lyric_runtime::Promise::State::Rejected:
                    numdone++;
                    break;
                default:
                    break;
            }
        }
        TU_LOG_INFO << numdone << " of " << numdeps << " completed";
        if (numdone == numdeps) {
            TU_LOG_INFO << "sending signal";
            done->sendSignal();
        }
    }
    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        TU_LOG_INFO << "all deps completed";
        promise->complete(lyric_runtime::DataCell(true));
        auto *loop = state->mainLoop();
        uv_stop(loop);
    }
};

void on_allof_accept(lyric_runtime::Promise *promise, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state)
{
    promise->complete(lyric_runtime::DataCell(true));
}

TEST_F (SystemScheduler, RegisterAllOfTarget)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = systemScheduler->systemLoop();

    auto timer1 = lyric_runtime::Promise::create(on_allof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(25, timer1), tempo_test::IsOk());

    auto timer2 = lyric_runtime::Promise::create(on_allof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(50, timer2), tempo_test::IsOk());

    auto timer3 = lyric_runtime::Promise::create(on_allof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(100, timer3), tempo_test::IsOk());

    auto ops = std::make_unique<AllOfOps>();
    auto target = lyric_runtime::Promise::create(std::move(ops));
    ASSERT_THAT (systemScheduler->registerTarget(target), tempo_test::IsOk());

    ASSERT_THAT (timer1->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer2->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer3->forward(target), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    std::vector timers{timer1->getState(), timer2->getState(), timer3->getState()};
    ASSERT_THAT (timers, ::testing::Each(lyric_runtime::Promise::State::Completed));
    ASSERT_EQ (lyric_runtime::Promise::State::Completed, target->getState());

    auto result = target->getResult();
    ASSERT_THAT (result, DataCellBool(true));
}

class AnyOfOps : public lyric_runtime::PromiseOperations {
public:
    void onPartial(
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsBegin,
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsEnd,
        std::shared_ptr<lyric_runtime::AsyncHandle> &done) override {
        int numdeps = 0;
        int numdone = 0;
        for (; depsBegin != depsEnd; ++depsBegin) {
            numdeps++;
            switch ((*depsBegin)->getState()) {
                case lyric_runtime::Promise::State::Completed:
                case lyric_runtime::Promise::State::Rejected:
                    numdone++;
                    break;
                default:
                    break;
            }
        }
        TU_LOG_INFO << numdone << " of " << numdeps << " completed";
        if (numdone > 0) {
            TU_LOG_INFO << "sending signal";
            done->sendSignal();
        }
    }
    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        TU_LOG_INFO << "any deps completed";
        promise->complete(lyric_runtime::DataCell(true));
        auto *loop = state->mainLoop();
        uv_stop(loop);
    }
};

void on_anyof_accept(lyric_runtime::Promise *promise, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state)
{
    promise->complete(lyric_runtime::DataCell(true));
}

TEST_F (SystemScheduler, RegisterAnyOfTarget)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = systemScheduler->systemLoop();

    auto timer1 = lyric_runtime::Promise::create(on_anyof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(25, timer1), tempo_test::IsOk());

    auto timer2 = lyric_runtime::Promise::create(on_anyof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(50, timer2), tempo_test::IsOk());

    auto timer3 = lyric_runtime::Promise::create(on_anyof_accept);
    ASSERT_THAT (systemScheduler->registerTimer(100, timer3), tempo_test::IsOk());

    auto ops = std::make_unique<AnyOfOps>();
    auto target = lyric_runtime::Promise::create(std::move(ops));
    ASSERT_THAT (systemScheduler->registerTarget(target), tempo_test::IsOk());

    ASSERT_THAT (timer1->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer2->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer3->forward(target), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    std::vector timers{timer1->getState(), timer2->getState(), timer3->getState()};
    ASSERT_THAT (timers, ::testing::Contains(lyric_runtime::Promise::State::Completed));
    ASSERT_EQ (lyric_runtime::Promise::State::Completed, target->getState());

    auto result = target->getResult();
    ASSERT_THAT (result, DataCellBool(true));
}

class FirstCompletedOps : public lyric_runtime::PromiseOperations {
public:
    void onPartial(
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsBegin,
        std::vector<std::shared_ptr<lyric_runtime::Promise>>::const_iterator depsEnd,
        std::shared_ptr<lyric_runtime::AsyncHandle> &done) override {
        int numdeps = 0;
        int numcompleted = 0;
        int numrejected = 0;
        for (; depsBegin != depsEnd; ++depsBegin) {
            const auto &promise = *depsBegin;
            numdeps++;
            switch (promise->getState()) {
                case lyric_runtime::Promise::State::Completed: {
                    if (promise->getResult() == lyric_runtime::DataCell(true)) {
                        numcompleted++;
                        break;
                    }
                    [[fallthrough]];
                }
                case lyric_runtime::Promise::State::Rejected:
                    numrejected++;
                    break;
                default:
                    break;
            }
        }
        TU_LOG_INFO << numcompleted << " completed, " << numrejected << " rejected out of " << numdeps << " total";
        if (numcompleted > 0) {
            TU_LOG_INFO << "sending signal";
            done->sendSignal();
        }
    }
    void onAccept(
        lyric_runtime::Promise *promise,
        const lyric_runtime::Waiter *waiter,
        lyric_runtime::InterpreterState *state) override
    {
        TU_LOG_INFO << "first dep completed";
        promise->complete(lyric_runtime::DataCell(true));
        auto *loop = state->mainLoop();
        uv_stop(loop);
    }
};

void on_first_completed_true(lyric_runtime::Promise *promise, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state)
{
    promise->complete(lyric_runtime::DataCell(true));
}

void on_first_completed_false(lyric_runtime::Promise *promise, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state)
{
    promise->complete(lyric_runtime::DataCell(false));
}

TEST_F (SystemScheduler, RegisterFirstCompletedTarget)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = systemScheduler->systemLoop();

    auto timer1 = lyric_runtime::Promise::create(on_first_completed_false);
    ASSERT_THAT (systemScheduler->registerTimer(25, timer1), tempo_test::IsOk());

    auto timer2 = lyric_runtime::Promise::create(on_first_completed_true);
    ASSERT_THAT (systemScheduler->registerTimer(50, timer2), tempo_test::IsOk());

    auto timer3 = lyric_runtime::Promise::create(on_first_completed_false);
    ASSERT_THAT (systemScheduler->registerTimer(100, timer3), tempo_test::IsOk());

    auto ops = std::make_unique<FirstCompletedOps>();
    auto target = lyric_runtime::Promise::create(std::move(ops));
    ASSERT_THAT (systemScheduler->registerTarget(target), tempo_test::IsOk());

    ASSERT_THAT (timer1->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer2->forward(target), tempo_test::IsOk());
    ASSERT_THAT (timer3->forward(target), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    std::vector timers{timer1->getState(), timer2->getState(), timer3->getState()};
    ASSERT_THAT (timers, ::testing::Contains(lyric_runtime::Promise::State::Completed));
    ASSERT_EQ (lyric_runtime::Promise::State::Completed, target->getState());

    auto result = target->getResult();
    ASSERT_THAT (result, DataCellBool(true));
}
