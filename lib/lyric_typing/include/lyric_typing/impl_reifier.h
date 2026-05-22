#ifndef LYRIC_TYPING_IMPL_REIFIER_H
#define LYRIC_TYPING_IMPL_REIFIER_H

#include <vector>

#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/type_contract.h>

#include "type_system.h"

namespace lyric_typing {

    class ImplReifier {
    public:
        explicit ImplReifier(lyric_assembler::ObjectState *state);
        explicit ImplReifier(TypeSystem *typeSystem);

        bool isValid() const;

        tempo_utils::Status initialize(lyric_assembler::ConceptSymbol *conceptSymbol);
        tempo_utils::Status initialize(const lyric_common::TypeDef &implType);

        lyric_assembler::ConceptSymbol *getConcept() const;

        tempo_utils::Status reifyNextImplArgument(const lyric_common::TypeDef &argumentType);
        tempo_utils::Status reifyAliasArgument(const lyric_assembler::BindingSymbol *bindingSymbol);
        tempo_utils::Result<lyric_assembler::TypeContract> reifyContract() const;

    private:
        lyric_assembler::ObjectState *m_state;

        lyric_assembler::ConceptSymbol *m_concept = nullptr;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        std::vector<size_t> m_implTemplateParametersIndex;
        bool m_initialized = false;

        std::vector<lyric_common::TypeDef> m_implArgumentTypes;
        std::vector<lyric_common::TypeDef> m_contractArgumentTypes;
    };
}

#endif // LYRIC_TYPING_IMPL_REIFIER_H
