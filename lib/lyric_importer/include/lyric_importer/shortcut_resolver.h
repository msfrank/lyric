#ifndef LYRIC_IMPORTER_SHORTCUT_RESOLVER_H
#define LYRIC_IMPORTER_SHORTCUT_RESOLVER_H

#include <absl/container/flat_hash_map.h>
#include <tempo_utils/result.h>

#include <tempo_utils/status.h>
#include <tempo_utils/url_origin.h>

namespace lyric_importer {

    class ShortcutResolver {
    public:
        ShortcutResolver() = default;

        bool hasShortcut(const std::string &shortcut) const;
        tempo_utils::UrlOrigin getShortcut(const std::string &shortcut) const;
        absl::flat_hash_map<std::string,tempo_utils::UrlOrigin>::const_iterator shortcutsBegin() const;
        absl::flat_hash_map<std::string,tempo_utils::UrlOrigin>::const_iterator shortcutsEnd() const;
        int numShortcuts() const;

        tempo_utils::Status insertShortcut(const std::string &shortcut, const tempo_utils::UrlOrigin &origin);
        tempo_utils::Result<tempo_utils::UrlOrigin> resolveShortcut(const std::string &shortcut) const;

    private:
        absl::flat_hash_map<std::string,tempo_utils::UrlOrigin> m_shortcuts;
    };
}

#endif // LYRIC_IMPORTER_SHORTCUT_RESOLVER_H
