#ifndef LYRIC_OBJECT_PLUGIN_SPECIFIER_H
#define LYRIC_OBJECT_PLUGIN_SPECIFIER_H

#include <filesystem>
#include <string>

#include <tempo_utils/integer_types.h>

namespace lyric_object {

    class PluginSpecifier {

    public:
        PluginSpecifier();
        PluginSpecifier(
            std::string_view systemName,
            std::string_view architecture,
            std::string_view systemVersion = {},
            std::string_view compilerId = {});
        PluginSpecifier(const PluginSpecifier &other);

        bool isValid() const;

        std::string getSystemName() const;
        std::string getArchitecture() const;
        std::string getSystemVersion() const;
        std::string getCompilerId() const;

        std::string toString() const;

        static PluginSpecifier systemDefault();

    private:
        std::string m_systemName;
        std::string m_architecture;
        std::string m_systemVersion;
        std::string m_compilerId;
    };
}

#endif // LYRIC_OBJECT_PLUGIN_SPECIFIER_H