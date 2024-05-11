#ifndef LYRIC_BUILD_TASK_HASHER_H
#define LYRIC_BUILD_TASK_HASHER_H

#include <filesystem>
#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include "build_types.h"
#include "sha256_hash.h"

namespace lyric_build {

    class TaskHasher {

    public:
        TaskHasher(const TaskKey &key);

        void hashValue(const TaskKey &key);
        void hashValue(bool b);
        void hashValue(int64_t i64);
        void hashValue(double dbl);
        void hashValue(const std::string_view &s);
        void hashValue(const std::vector<std::string> &sl);
        bool hashFile(const std::filesystem::path &path);
        std::string finish();

        static std::string uniqueHash();

    private:
        Sha256Hash m_hasher;
    };
}

#endif // LYRIC_BUILD_TASK_HASHER_H
