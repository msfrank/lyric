#ifndef LYRIC_ASSEMBLER_PACK_BUILDER_H
#define LYRIC_ASSEMBLER_PACK_BUILDER_H

#include "assembler_types.h"

namespace lyric_assembler {

    class PackBuilder {
    public:
        PackBuilder();
        PackBuilder(const PackBuilder &other) = delete;

        tempo_utils::Status appendListParameter(
            std::string_view name,
            std::string_view label,
            const lyric_common::TypeDef &type,
            bool isVariable);
        tempo_utils::Status appendListOptParameter(
            std::string_view name,
            std::string_view label,
            const lyric_common::TypeDef &type,
            bool isVariable);
        tempo_utils::Status appendNamedParameter(
            std::string_view name,
            std::string_view label,
            const lyric_common::TypeDef &type,
            bool isVariable);
        tempo_utils::Status appendNamedOptParameter(
            std::string_view name,
            std::string_view label,
            const lyric_common::TypeDef &type,
            bool isVariable);
        tempo_utils::Status appendCtxParameter(
            std::string_view name,
            std::string_view label,
            const lyric_common::TypeDef &type);
        tempo_utils::Status appendRestParameter(
            std::string_view name,
            const lyric_common::TypeDef &type);

        tempo_utils::Result<ParameterPack> toParameterPack() const;

    private:
        std::vector<Parameter> m_listParameters;
        std::vector<Parameter> m_namedParameters;
        Option<Parameter> m_restParameter;
        absl::flat_hash_set<std::string> m_usedNames;
        absl::flat_hash_set<std::string> m_usedLabels;
        tu_uint8 m_currindex;
        tu_int16 m_firstlistopt;

        tempo_utils::Status check(std::string_view name, std::string_view label);
    };
}

#endif // LYRIC_ASSEMBLER_PACK_BUILDER_H
