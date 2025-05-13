
#include <absl/strings/str_cat.h>

#include <lyric_common/plugin.h>

std::string
lyric_common::pluginFilename(std::string_view pluginName)
{
    return absl::StrCat(pluginName, ".", pluginPlatformId(), pluginFileDotSuffix());
}

const char *
lyric_common::pluginFileSuffix()
{
    auto *dotSuffix = pluginFileDotSuffix();
    return ++dotSuffix;
}

const char *
lyric_common::pluginFileDotSuffix()
{
    return PLUGIN_SUFFIX;
}

const char *
lyric_common::pluginPlatformId()
{
    return PLUGIN_SYSTEM_NAME "-" PLUGIN_ARCHITECTURE;
}

const char *
lyric_common::pluginInitFunction()
{
    return "native_init";
}