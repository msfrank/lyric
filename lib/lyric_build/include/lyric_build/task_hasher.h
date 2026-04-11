#ifndef LYRIC_BUILD_TASK_HASHER_H
#define LYRIC_BUILD_TASK_HASHER_H

#include <filesystem>
#include <string>
#include <vector>

#include <tempo_security/sha256_hash.h>

#include "build_types.h"

namespace lyric_build {
    class BaseTask;

    class TaskHasher {

    public:
        explicit TaskHasher(const TaskKey &key);

        void hashValue(bool b);
        void hashValue(int64_t i64);
        void hashValue(double dbl);
        void hashValue(std::string_view sv);
        void hashValue(std::span<const tu_uint8> sp);
        void hashValue(const std::vector<std::string> &sl);
        tempo_utils::Status hashFile(const std::filesystem::path &path);
        void hashTask(const TaskKey &key, const TaskData &data);
        void hashTask(const BaseTask *task);

        TaskHash finish();

        static TaskHash uniqueHash();

    private:
        tempo_security::Sha256Hash m_hasher;
    };
}

#endif // LYRIC_BUILD_TASK_HASHER_H
