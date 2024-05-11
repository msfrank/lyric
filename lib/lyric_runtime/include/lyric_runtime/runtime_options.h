#ifndef LYRIC_RUNTIME_RUNTIME_OPTIONS_H
#define LYRIC_RUNTIME_RUNTIME_OPTIONS_H

#include <string>

namespace lyric_runtime {

    struct RuntimeOptions {

        std::string bootDirectoryPath;

        // workspace config options
        std::string workspaceSrcDirectoryPath;
        std::string workspaceCacheDirectoryPath;
        std::string workspaceLibDirectoryPath;

        // distribution config options
        std::string distributionLibDirectoryPath;
    };
}

#endif // LYRIC_RUNTIME_RUNTIME_OPTIONS_H
