#ifndef LYRIC_BUILD_SHA256_HASH_H
#define LYRIC_BUILD_SHA256_HASH_H

#include <absl/strings/string_view.h>

namespace lyric_build {

    class Sha256Hash {

    public:
        Sha256Hash();
        ~Sha256Hash();

        bool addData(std::string_view data);
        std::string getResult();

        static std::string hash(std::string_view data);

    private:
        void *m_ctx;
        bool m_dirty;
    };
}

#endif // LYRIC_BUILD_SHA256_HASH_H
