#ifndef LYRIC_BUILD_TASK_HASHER_H
#define LYRIC_BUILD_TASK_HASHER_H

#include <filesystem>
#include <string>
#include <vector>

#include <tempo_security/sha256_hash.h>

#include "build_types.h"

namespace lyric_build {

    class TaskHasher {

    public:
        explicit TaskHasher(const TaskKey &key);

        void hashValue(const TaskKey &key);
        void hashValue(bool b);
        void hashValue(int64_t i64);
        void hashValue(double dbl);
        void hashValue(const std::string_view &s);
        void hashValue(const std::vector<std::string> &sl);
        tempo_utils::Status hashFile(const std::filesystem::path &path);
        std::string finish();

        static std::string uniqueHash();

    private:
        tempo_security::Sha256Hash m_hasher;
    };
}

#endif // LYRIC_BUILD_TASK_HASHER_H
