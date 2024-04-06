#ifndef LYRIC_COMMON_SYMBOL_URL_H
#define LYRIC_COMMON_SYMBOL_URL_H

#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include <tempo_utils/log_message.h>
#include <tempo_utils/url.h>

#include "assembly_location.h"
#include "symbol_path.h"

namespace lyric_common {

    class SymbolUrl {
    public:
        SymbolUrl();
        explicit SymbolUrl(const SymbolPath &path);
        SymbolUrl(const AssemblyLocation &location, const SymbolPath &path);
        SymbolUrl(const SymbolUrl &other);
        SymbolUrl(SymbolUrl &&other) noexcept;

        SymbolUrl &operator=(const SymbolUrl &other);
        SymbolUrl &operator=(SymbolUrl &&other) noexcept;

        bool isValid() const;
        bool isAbsolute() const;
        bool isRelative() const;

        AssemblyLocation getAssemblyLocation() const;
        SymbolPath getSymbolPath() const;
        std::string getSymbolName() const;

        std::string toString() const;
        tempo_utils::Url toUrl() const;

        bool operator==(const SymbolUrl &other) const;
        bool operator!=(const SymbolUrl &other) const;

        static SymbolUrl fromString(std::string_view s);
        static SymbolUrl fromUrl(const tempo_utils::Url &uri);

        template <typename H>
        friend H AbslHashValue(H h, const SymbolUrl &url) {
            return H::combine(std::move(h), url.m_location, url.m_path);
        }

    private:
        AssemblyLocation m_location;
        SymbolPath m_path;
    };

    bool operator<(const SymbolUrl &lhs, const SymbolUrl &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const SymbolUrl &symbolUrl);
}

#endif // LYRIC_COMMON_SYMBOL_URL_H