#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_build/memory_cache.h>
#include <lyric_runtime/static_loader.h>
#include <tempo_test/result_matchers.h>

#include "test_task.h"


void on_notification(
    lyric_build::BuildRunner *runner,
    const lyric_build::TaskNotification *notification,
    void *data)
{
}

class BuildRunner : public ::testing::Test {
protected:
    lyric_build::BuildGeneration buildgen;
    std::shared_ptr<lyric_build::AbstractCache> cache;
    std::shared_ptr<lyric_build::BuildState> state;
    lyric_build::TaskSettings taskSettings;
    lyric_build::TaskRegistry taskRegistry;
    std::unique_ptr<lyric_build::BuildRunner> runner;


    void SetUp() override {
        std::shared_ptr<lyric_build::AbstractFilesystem> vfs;
        TU_ASSIGN_OR_RAISE (vfs, lyric_build::LocalFilesystem::create(std::filesystem::current_path()));

        buildgen = lyric_build::BuildGeneration::create();
        cache = std::make_shared<lyric_build::MemoryCache>();
        auto emptyLoader = std::make_shared<lyric_runtime::StaticLoader>();
        state = std::make_shared<lyric_build::BuildState>(buildgen,
            std::static_pointer_cast<lyric_build::AbstractCache>(cache),
            std::make_shared<lyric_bootstrap::BootstrapLoader>(),
            std::shared_ptr<lyric_runtime::AbstractLoader>{},
            lyric_importer::ModuleCache::create(emptyLoader),
            std::make_shared<lyric_importer::ShortcutResolver>(),
            vfs,
            std::filesystem::current_path());

        runner = std::make_unique<lyric_build::BuildRunner>(
            &taskSettings, state, cache, &taskRegistry, 1, 1000, on_notification, nullptr);
    }
};

TEST_F(BuildRunner, WaitForNextReadyTimesOut)
{
    auto readyItem = runner->waitForNextReady(3);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TIMEOUT, readyItem.type);
}

TEST_F(BuildRunner, EnqueueNewTask)
{
    taskRegistry.registerTaskDomain("test", new_test_task);
    taskRegistry.sealRegistry();

    lyric_build::TaskKey key("test", std::string{"foo"});
    auto enqueueTaskStatus = runner->enqueueTask(key, {});
    ASSERT_THAT (enqueueTaskStatus, tempo_test::IsOk());

    auto readyItem = runner->waitForNextReady(0);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TASK, readyItem.type);
    ASSERT_EQ (key, readyItem.task->getKey());

    auto taskState = state->loadState(key);
    ASSERT_EQ (lyric_build::TaskState::Status::QUEUED, taskState.getStatus());
    ASSERT_EQ (tempo_utils::UUID{}, taskState.getGeneration());
    ASSERT_EQ ("", taskState.getHash());
}

TEST_F(BuildRunner, EnqueueExistingTaskReturnsExistingTask)
{
    taskRegistry.registerTaskDomain("test", new_test_task);
    taskRegistry.sealRegistry();

    lyric_build::TaskKey key("test", std::string{"foo"});
    auto enqueueTaskStatus1 = runner->enqueueTask(key, {});
    ASSERT_THAT (enqueueTaskStatus1, tempo_test::IsOk());

    auto readyItem1 = runner->waitForNextReady(0);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TASK, readyItem1.type);
    ASSERT_EQ (key, readyItem1.task->getKey());
    auto *task1 = readyItem1.task;

    auto enqueueTaskStatus2 = runner->enqueueTask(key, {});
    ASSERT_THAT (enqueueTaskStatus2, tempo_test::IsOk());

    auto readyItem2 = runner->waitForNextReady(0);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TASK, readyItem2.type);
    auto *task2 = readyItem2.task;

    ASSERT_EQ (task1, task2);
}

TEST_F(BuildRunner, EnqueueExistingTaskUpdatesTaskState)
{
    taskRegistry.registerTaskDomain("test", new_test_task);
    taskRegistry.sealRegistry();

    lyric_build::TaskKey key("test", std::string{"foo"});

    auto priorGen = tempo_utils::UUID::randomUUID();
    std::string priorHash("foo");
    lyric_build::TaskState priorState(lyric_build::TaskState::Status::BLOCKED, priorGen, priorHash);
    state->storeState(key, priorState);

    auto enqueueTaskStatus1 = runner->enqueueTask(key, {});
    ASSERT_THAT (enqueueTaskStatus1, tempo_test::IsOk());

    auto readyItem = runner->waitForNextReady(0);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TASK, readyItem.type);
    ASSERT_EQ (key, readyItem.task->getKey());

    auto newState = state->loadState(key);
    ASSERT_EQ (lyric_build::TaskState::Status::QUEUED, newState.getStatus());
    ASSERT_EQ (priorGen, newState.getGeneration());
    ASSERT_EQ (priorHash, newState.getHash());
}

TEST_F(BuildRunner, ShutdownEnqueuesShutdownForEachThread) {

    int numThreads = 4;
    // create new runner with multiple threads
    runner = std::make_unique<lyric_build::BuildRunner>(
        &taskSettings, state, cache, &taskRegistry, numThreads, 1000, on_notification, nullptr);

    runner->shutdown();

    // we expect a shutdown item for each thread
    for (int i = 0; i < numThreads; i++) {
        auto readyItem = runner->waitForNextReady(0);
        ASSERT_EQ (lyric_build::ReadyItem::Type::SHUTDOWN, readyItem.type);
    }

    // no more shutdown items
    auto readyItem = runner->waitForNextReady(0);
    ASSERT_EQ (lyric_build::ReadyItem::Type::TIMEOUT, readyItem.type);
}