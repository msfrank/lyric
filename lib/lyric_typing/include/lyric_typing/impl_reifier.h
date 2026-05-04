#ifndef LYRIC_TYPING_IMPL_REIFIER_H
#define LYRIC_TYPING_IMPL_REIFIER_H

#include <vector>

#include <lyric_assembler/object_state.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_common/symbol_url.h>

#include "type_system.h"

namespace lyric_typing {

    class ImplReifier {
    public:
        ImplReifier();
        explicit ImplReifier(lyric_assembler::ObjectState *state);
        explicit ImplReifier(TypeSystem *typeSystem);

        bool isValid() const;

        tempo_utils::Status initialize(const lyric_assembler::ConceptSymbol *conceptSymbol);
        tempo_utils::Status initialize(const lyric_common::TypeDef &implType);

        tempo_utils::Status reifyNextImplArgument(const lyric_common::TypeDef &argumentType);
        tempo_utils::Result<lyric_common::TypeDef> reifyImplType() const;

        tempo_utils::Status reifyAliasArgument(const lyric_assembler::BindingSymbol *bindingSymbol);
        tempo_utils::Result<lyric_common::TypeDef> reifyContractType() const;

    private:
        lyric_assembler::ObjectState *m_state;

        lyric_common::SymbolUrl m_conceptUrl;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        std::vector<size_t> m_implTemplateParametersIndex;
        bool m_initialized;

        std::vector<lyric_common::TypeDef> m_implArgumentTypes;
        std::vector<lyric_common::TypeDef> m_contractArgumentTypes;

        tempo_utils::Status checkPlaceholder(
            const lyric_object::TemplateParameter &tp,
            const lyric_common::TypeDef &arg) const;
    };
}

#endif // LYRIC_TYPING_IMPL_REIFIER_H
