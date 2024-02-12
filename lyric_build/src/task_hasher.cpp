#include <random>

#include <lyric_build/task_hasher.h>
#include <lyric_build/build_types.h>
#include <tempo_utils/file_reader.h>

lyric_build::TaskHasher::TaskHasher(const TaskKey &key)
{
    // initialize hasher with task domain and task id (but not params)
    m_hasher.addData(key.getDomain());
    m_hasher.addData(key.getId());
}

void
lyric_build::TaskHasher::hashValue(const TaskKey &key)
{
    m_hasher.addData(key.toString());
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
lyric_build::TaskHasher::hashValue(const std::string_view &s)
{
    m_hasher.addData(s);
}

void
lyric_build::TaskHasher::hashValue(const std::vector<std::string> &sl)
{
    for (const auto &s : sl) {
        m_hasher.addData(s);
    }
}

bool
lyric_build::TaskHasher::hashFile(const std::filesystem::path &path)
{
    tempo_utils::FileReader reader(path.string());
    if (!reader.isValid())
        return false;
    auto bytes = reader.getBytes();
    m_hasher.addData(std::string_view((const char *) bytes->getData(), bytes->getSize()));
    return true;
}

std::string
lyric_build::TaskHasher::finish()
{
    return m_hasher.getResult();
}

std::string
lyric_build::TaskHasher::uniqueHash()
{
    std::mt19937 randengine{std::random_device()()};
    std::uniform_int_distribution<tu_uint64> chargen;

    std::string result;
    for (int i = 0; i < 4; i++) {
        // FIXME: use absl random functions instead, use a single thread local instance
        auto rd = chargen(randengine);
        const tu_uint8 *bytes = (const tu_uint8 *) &rd;
        result.append((const char *) bytes, 8);
    }
    return result;
}
