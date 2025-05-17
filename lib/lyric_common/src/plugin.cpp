
#include <absl/strings/str_cat.h>

#include <lyric_common/plugin.h>
#include <tempo_utils/platform.h>

std::string
lyric_common::pluginFilename(std::string_view pluginName)
{
    return absl::StrCat(
        pluginName,
        ".",
        tempo_utils::sharedLibraryPlatformId(),
        tempo_utils::sharedLibraryFileDotSuffix());
}

const char *
lyric_common::pluginInitFunction()
{
    return "native_init";
}