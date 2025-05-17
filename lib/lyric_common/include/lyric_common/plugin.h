#ifndef LYRIC_COMMON_PLUGIN_H
#define LYRIC_COMMON_PLUGIN_H

#include <string>

namespace lyric_common {

    std::string pluginFilename(std::string_view pluginName);

    const char *pluginInitFunction();

}

#endif // LYRIC_COMMON_PLUGIN_H
