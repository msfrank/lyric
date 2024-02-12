#ifndef LYRIC_COMMON_SYMBOL_PATH_H
#define LYRIC_COMMON_SYMBOL_PATH_H

#include <string>
#include <vector>

#include <absl/strings/string_view.h>

#include <tempo_utils/log_message.h>
#include <tempo_utils/prehashed_value.h>
#include <tempo_utils/url.h>

#include "assembly_location.h"

namespace lyric_common {

    class SymbolPath {
    public:
        SymbolPath();
        explicit SymbolPath(const std::vector<std::string> &symbolPath);
        explicit SymbolPath(std::initializer_list<std::string> &symbolPath);
        SymbolPath(const std::vector<std::string> &symbolEnclosure, std::string_view symbolName);
        SymbolPath(std::initializer_list<std::string> &symbolEnclosure, std::string_view symbolName);
        SymbolPath(const SymbolPath &other);
        SymbolPath(SymbolPath &&other) noexcept;

        SymbolPath &operator=(const SymbolPath &other);
        SymbolPath &operator=(SymbolPath &&other) noexcept;

        bool isValid() const;
        std::vector<std::string> getPath() const;
        std::vector<std::string> getEnclosure() const;
        std::string getName() const;

        std::string toString() const;

        bool operator==(const SymbolPath &other) const;
        bool operator!=(const SymbolPath &other) const;

        static SymbolPath fromString(const std::string &string);

        static SymbolPath entrySymbol();

    private:
        struct Priv {
            template <typename H>
            friend H AbslHashValue(H h, const std::vector<std::string> &parts_) {
                return H::combine_contiguous(std::move(h), parts_.data(), parts_.size());
            }
            tempo_utils::PrehashedValue<std::vector<std::string>> parts;
            Priv() {};
            Priv(std::vector<std::string> &&parts_) : parts(std::move(parts_)) {};
            Priv(std::initializer_list<std::string>::iterator begin, std::initializer_list<std::string>::iterator end)
                : parts(tempo_utils::make_prehashed<std::vector<std::string>>(begin, end)) {};
            Priv(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end)
                : parts(tempo_utils::make_prehashed<std::vector<std::string>>(begin, end)) {};
        };
        std::shared_ptr<const Priv> m_priv;

    public:
        template <typename H>
        friend H AbslHashValue(H h, const SymbolPath &path) {
            return H::combine(std::move(h), path.m_priv->parts);
        }
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const SymbolPath &symbolPath);
}

#endif // LYRIC_COMMON_SYMBOL_PATH_H