#ifndef LYRIC_ASSEMBLER_IMPL_CACHE_H
#define LYRIC_ASSEMBLER_IMPL_CACHE_H

#include <lyric_importer/impl_import.h>

#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    // forward declarations
    class ImplHandle;

    class ImplCache {
    public:
        explicit ImplCache(ObjectState *objectState);
        ~ImplCache();

        tempo_utils::Result<ImplHandle *> makeImpl(
            const lyric_common::TypeDef &consumerType,
            const lyric_common::SymbolUrl &receiverUrl,
            bool isDeclOnly,
            BlockHandle *parentBlock);
        tempo_utils::Result<ImplHandle *> makeImpl(
            const lyric_common::TypeDef &consumerType,
            const lyric_common::SymbolUrl &receiverUrl,
            TemplateHandle *receiverTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock);

        tempo_utils::Result<ImplHandle *> importImpl(std::shared_ptr<lyric_importer::ImplImport> implImport);

        std::vector<ImplHandle *>::const_iterator implsBegin() const;
        std::vector<ImplHandle *>::const_iterator implsEnd() const;
        int numImpls() const;

        tempo_utils::Result<ImplHandle *> getOrImportImpl(
            const ImplReference &implRef,
            bool allowMissing = false) const;
        tempo_utils::Result<ImplHandle *> getOrImportImpl(
            const lyric_common::SymbolUrl &symbolUrl,
            const lyric_common::TypeDef &implType,
            bool allowMissing = false) const;

        tempo_utils::Result<CallSymbol *> getOrImportImplMethod(
            const ImplReference &implRef,
            const ActionSymbol *actionSymbol,
            bool allowMissing = false) const;
        tempo_utils::Result<CallSymbol *> getOrImportImplMethod(
            const lyric_common::SymbolUrl &symbolUrl,
            const lyric_common::TypeDef &implType,
            const ActionSymbol *actionSymbol,
            bool allowMissing = false) const;

        bool hasEnvImpl(const lyric_common::TypeDef &type) const;
        lyric_common::SymbolUrl getEnvImpl(const lyric_common::TypeDef &type) const;
        tempo_utils::Status insertEnvImpl(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url);

    private:
        ObjectState *m_objectState;
        std::vector<ImplHandle *> m_importedImpls;
        std::vector<ImplHandle *> m_declaredImpls;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_envImpls;
    };
}

#endif // LYRIC_ASSEMBLER_IMPL_CACHE_H
