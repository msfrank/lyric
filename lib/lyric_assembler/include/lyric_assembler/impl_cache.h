#ifndef LYRIC_ASSEMBLER_IMPL_CACHE_H
#define LYRIC_ASSEMBLER_IMPL_CACHE_H

#include <lyric_importer/impl_import.h>

#include "assembler_tracer.h"
#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    // forward declarations
    class ImplHandle;

    class ImplCache {
    public:
        ImplCache(ObjectState *objectState, AssemblerTracer *tracer);
        ~ImplCache();

        tempo_utils::Result<ImplHandle *> makeImpl(
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            bool isDeclOnly,
            BlockHandle *parentBlock);
        tempo_utils::Result<ImplHandle *> makeImpl(
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            TemplateHandle *receiverTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock);

        tempo_utils::Result<ImplHandle *> importImpl(lyric_importer::ImplImport *implImport);

        std::vector<ImplHandle *>::const_iterator implsBegin() const;
        std::vector<ImplHandle *>::const_iterator implsEnd() const;
        int numImpls() const;

        bool hasEnvImpl(const lyric_common::TypeDef &type) const;
        lyric_common::SymbolUrl getEnvImpl(const lyric_common::TypeDef &type) const;
        tempo_utils::Status insertEnvImpl(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url);

    private:
        ObjectState *m_objectState;
        AssemblerTracer *m_tracer;
        std::vector<ImplHandle *> m_importedImpls;
        std::vector<ImplHandle *> m_declaredImpls;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_envImpls;
    };
}

#endif // LYRIC_ASSEMBLER_IMPL_CACHE_H
