#ifndef LYRIC_COMMON_MODULE_LOCATION_H
#define LYRIC_COMMON_MODULE_LOCATION_H

#include <vector>
#include <string>

#include <tempo_utils/log_message.h>
#include <tempo_utils/prehashed_value.h>
#include <tempo_utils/url.h>

namespace lyric_common {

    class ModuleLocation {
    public:
        ModuleLocation();
        ModuleLocation(const ModuleLocation &other);
        ModuleLocation(ModuleLocation &&other) noexcept;

        ModuleLocation &operator=(const ModuleLocation &other);
        ModuleLocation &operator=(ModuleLocation &&other) noexcept;

        bool isValid() const;
        bool isAbsolute() const;
        bool isRelative() const;

        bool hasScheme() const;
        bool hasOrigin() const;
        bool hasAuthority() const;
        bool hasPathParts() const;

        std::string getScheme() const;
        tempo_utils::UrlOrigin getOrigin() const;
        tempo_utils::UrlAuthority getAuthority() const;
        tempo_utils::UrlPath getPath() const;
        std::string getModuleName() const;

        ModuleLocation resolve(const ModuleLocation &rel) const;

        std::string toString() const;
        tempo_utils::Url toUrl() const;

        bool operator==(const ModuleLocation &other) const;
        bool operator!=(const ModuleLocation &other) const;

        static ModuleLocation fromString(std::string_view s);
        static ModuleLocation fromUrl(const tempo_utils::Url &url);
        static ModuleLocation fromUrlPath(const tempo_utils::UrlPath &urlPath);

        template<typename H>
        friend H AbslHashValue(H h, const ModuleLocation &location) {
            return H::combine(std::move(h), location.m_location);
        }

    private:
        tempo_utils::PrehashedValue<tempo_utils::Url> m_location;

        explicit ModuleLocation(const tempo_utils::Url &location);
        explicit ModuleLocation(std::string_view path);
        ModuleLocation(std::string_view origin, std::string_view path);
    };

    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const ModuleLocation &location);
}

#endif // LYRIC_COMMON_MODULE_LOCATION_H