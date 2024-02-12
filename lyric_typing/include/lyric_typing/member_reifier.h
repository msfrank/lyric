#ifndef LYRIC_TYPING_MEMBER_REIFIER_H
#define LYRIC_TYPING_MEMBER_REIFIER_H

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <lyric_common/symbol_url.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/assembler_types.h>

#include "type_system.h"

namespace lyric_typing {

    class MemberReifier : public lyric_assembler::AbstractMemberReifier {

    public:
        MemberReifier();
        MemberReifier(
            TypeSystem *typeSystem,
            const lyric_common::TypeDef &receiverType,
            const lyric_assembler::TemplateHandle *templateHandle = nullptr);
        MemberReifier(
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            const std::vector<lyric_common::TypeDef> &templateArguments,
            TypeSystem *typeSystem);

        bool isValid() const override;

        tempo_utils::Result<lyric_assembler::SymbolBinding> reifyMember(
            const std::string &name,
            const lyric_assembler::FieldSymbol *fieldSymbol) override;

    private:
        lyric_common::SymbolUrl m_templateUrl;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        std::vector<lyric_common::TypeDef> m_reifiedPlaceholders;
        absl::flat_hash_map<std::string, lyric_assembler::SymbolBinding> m_memberCache;
        TypeSystem *m_typeSystem;

        tempo_utils::Status checkPlaceholder(
            const lyric_object::TemplateParameter &tp,
            const lyric_common::TypeDef &arg) const;
        tempo_utils::Result<lyric_common::TypeDef> reifySingular(
            const lyric_common::TypeDef &paramType);
        tempo_utils::Result<lyric_common::TypeDef> reifyUnion(
            const lyric_common::TypeDef &paramType);
    };
}

#endif // LYRIC_TYPING_MEMBER_REIFIER_H
