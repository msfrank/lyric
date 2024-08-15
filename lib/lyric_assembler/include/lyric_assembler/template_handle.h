#ifndef LYRIC_ASSEMBLER_TEMPLATE_HANDLE_H
#define LYRIC_ASSEMBLER_TEMPLATE_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>
#include <lyric_common/type_def.h>
#include <lyric_object/object_types.h>

#include "block_handle.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class TemplateHandle : public AbstractResolver {

    public:
        explicit TemplateHandle(ObjectState *state);
        TemplateHandle(
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            TemplateAddress address,
            TemplateHandle *superTemplate,
            BlockHandle *parentBlock,
            ObjectState *state);
        TemplateHandle(
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            TemplateAddress address,
            BlockHandle *parentBlock,
            ObjectState *state);
        TemplateHandle(
            const lyric_common::SymbolUrl &templateUrl,
            TemplateHandle *superTemplate,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            ObjectState *state);

        tempo_utils::Result<lyric_common::TypeDef> resolveSingular(
            const lyric_common::SymbolPath &typePath,
            const std::vector<lyric_common::TypeDef> &typeArguments) override;

        lyric_common::SymbolUrl getTemplateUrl() const;
        TemplateAddress getAddress() const;
        TemplateHandle *superTemplate() const;

        bool hasTemplateParameter(const std::string &name) const;
        lyric_object::TemplateParameter getTemplateParameter(const std::string &name) const;
        lyric_object::TemplateParameter getTemplateParameter(int index) const;
        std::vector<lyric_object::TemplateParameter> getTemplateParameters() const;
        std::vector<lyric_object::TemplateParameter>::const_iterator templateParametersBegin() const;
        std::vector<lyric_object::TemplateParameter>::const_iterator templateParametersEnd() const;
        int numTemplateParameters() const;

        lyric_common::TypeDef getPlaceholder(int index) const;
        lyric_common::TypeDef getPlaceholder(const std::string &name) const;
        std::vector<lyric_common::TypeDef> getPlaceholders() const;
        int numPlaceholders() const;

        void touch();

    private:
        lyric_common::SymbolUrl m_templateUrl;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        absl::flat_hash_map<std::string, int> m_parameterIndex;
        std::vector<lyric_common::TypeDef> m_placeholders;
        TemplateAddress m_address;
        TemplateHandle *m_superTemplate;
        BlockHandle *m_parentBlock;
        ObjectState *m_state;

        lyric_common::TypeDef resolvePlaceholder(
            std::string_view name,
            const std::vector<lyric_common::TypeDef> &typeArguments) const;
    };
}

#endif // LYRIC_ASSEMBLER_TEMPLATE_HANDLE_H
