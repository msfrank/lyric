#include <random>

#include <lyric_build/base_task.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_hasher.h>
#include <tempo_utils/file_reader.h>

lyric_build::TaskHasher::TaskHasher(const TaskKey &key)
{
    // initialize hasher with task domain and task id (but not params)
    m_hasher.addData(key.getDomain());
    m_hasher.addData(key.getId());
}

void
lyric_build::TaskHasher::hashValue(bool b)
{
    char c = b? 1 : 0;
    m_hasher.addData(std::string_view(&c, 1));
}

void
lyric_build::TaskHasher::hashValue(int64_t i64)
{
    m_hasher.addData(std::string_view((const char *)&i64, sizeof(int64_t)));
}

void
lyric_build::TaskHasher::hashValue(double dbl)
{
    m_hasher.addData(std::string_view((const char *)&dbl, sizeof(double)));
}

void
lyric_build::TaskHasher::hashValue(std::string_view sv)
{
    m_hasher.addData(sv);
}

void
lyric_build::TaskHasher::hashValue(std::span<const tu_uint8> sp)
{
    m_hasher.addData(sp);
}

void
lyric_build::TaskHasher::hashValue(const std::vector<std::string> &sl)
{
    for (const auto &s : sl) {
        m_hasher.addData(s);
    }
}

tempo_utils::Status
lyric_build::TaskHasher::hashFile(const std::filesystem::path &path)
{
    tempo_utils::FileReader reader(path.string());
    TU_RETURN_IF_NOT_OK (reader.getStatus());
    auto bytes = reader.getBytes();
    m_hasher.addData(std::string_view((const char *) bytes->getData(), bytes->getSize()));
    return {};
}

void
lyric_build::TaskHasher::hashTask(const TaskKey &key, const TaskData &data)
{
    m_hasher.addData(key.toString());
    auto hash = data.getHash();
    m_hasher.addData(hash.bytesView());
}

void
lyric_build::TaskHasher::hashTask(const BaseTask *task)
{
    TU_NOTNULL (task);
    for (auto it = task->completedBegin(); it != task->completedEnd(); it++) {
        hashTask(it->first, it->second);
    }
}

lyric_build::TaskHash
lyric_build::TaskHasher::finish()
{
    auto result = m_hasher.getResult();
    std::vector<tu_uint8> bytes(result.size());
    memcpy(bytes.data(), result.data(), result.size());
    return TaskHash(std::move(bytes));
}

thread_local std::mt19937 task_hasher_randengine{std::random_device()()};

lyric_build::TaskHash
lyric_build::TaskHasher::uniqueHash()
{
    std::uniform_int_distribution<tu_uint8> chargen;

    std::vector<tu_uint8> bytes(32);
    for (int i = 0; i < 32; i++) {
        bytes[i] = chargen(task_hasher_randengine);
    }
    return TaskHash(std::move(bytes));
}
