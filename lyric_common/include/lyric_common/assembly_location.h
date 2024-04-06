#ifndef LYRIC_COMMON_ASSEMBLY_LOCATION_H
#define LYRIC_COMMON_ASSEMBLY_LOCATION_H

#include <vector>
#include <string>

#include <tempo_utils/log_message.h>
#include <tempo_utils/prehashed_value.h>
#include <tempo_utils/url.h>

namespace lyric_common {

    class AssemblyLocation {
    public:
        AssemblyLocation();
        AssemblyLocation(std::string_view path);
        AssemblyLocation(std::string_view origin, std::string_view path);
        AssemblyLocation(const AssemblyLocation &other);
        AssemblyLocation(AssemblyLocation &&other) noexcept;

        AssemblyLocation &operator=(const AssemblyLocation &other);
        AssemblyLocation &operator=(AssemblyLocation &&other) noexcept;

        bool isValid() const;

        bool hasScheme() const;
        bool hasOrigin() const;
        bool hasAuthority() const;
        bool hasPathParts() const;

        std::string getScheme() const;
        tempo_utils::UrlOrigin getOrigin() const;
        tempo_utils::UrlAuthority getAuthority() const;
        tempo_utils::UrlPath getPath() const;

        std::string getAssemblyName() const;

        std::string toString() const;
        tempo_utils::Url toUrl() const;

        bool operator==(const AssemblyLocation &other) const;
        bool operator!=(const AssemblyLocation &other) const;

        static AssemblyLocation fromString(std::string_view s);
        static AssemblyLocation fromUrl(const tempo_utils::Url &uri);

        template<typename H>
        friend H AbslHashValue(H h, const AssemblyLocation &location) {
            return H::combine(std::move(h), location.m_location);
        }

    private:
        tempo_utils::PrehashedValue<tempo_utils::Url> m_location;

        AssemblyLocation(const tempo_utils::Url &location);
    };

    tempo_utils::LogMessage &&operator<<(tempo_utils::LogMessage &&message, const AssemblyLocation &location);
}

#endif // LYRIC_COMMON_ASSEMBLY_LOCATION_H