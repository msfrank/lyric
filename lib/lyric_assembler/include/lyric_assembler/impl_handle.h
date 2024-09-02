#ifndef LYRIC_ASSEMBLER_IMPL_HANDLE_H
#define LYRIC_ASSEMBLER_IMPL_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/impl_import.h>

#include "abstract_callable.h"
#include "assembler_types.h"
#include "object_state.h"
#include "base_handle.h"
#include "block_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ImplHandlePriv {
        ImplRef ref;
        std::string name;
        bool isDeclOnly = false;
        TypeHandle *implType = nullptr;
        ConceptSymbol *implConcept = nullptr;
        lyric_common::SymbolUrl receiverUrl;
        TemplateHandle *receiverTemplate = nullptr;
        absl::flat_hash_map<std::string, ExtensionMethod> extensions;
        std::unique_ptr<BlockHandle> implBlock;
    };

    class ImplHandle : public BaseHandle<ImplHandlePriv> {
    public:
        ImplHandle(
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        ImplHandle(
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            TemplateHandle *receiverTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ImplHandle(lyric_importer::ImplImport *implImport, ObjectState *state);

        ImplRef getRef() const;
        std::string getName() const;

        bool isDeclOnly() const;

        TypeHandle *implType() const;
        ConceptSymbol *implConcept() const;
        BlockHandle *implBlock() const;

        lyric_common::SymbolUrl getReceiverUrl() const;

        /*
         * impl extension management
         */
        bool hasExtension(const std::string &name) const;
        Option<ExtensionMethod> getExtension(const std::string &name) const;
        absl::flat_hash_map<std::string, ExtensionMethod>::const_iterator methodsBegin() const;
        absl::flat_hash_map<std::string, ExtensionMethod>::const_iterator methodsEnd() const;
        tu_uint32 numExtensions() const;

        tempo_utils::Result<ProcHandle *> defineExtension(
            const std::string &name,
            const ParameterPack &parameterPack,
            const lyric_common::TypeDef &returnType);

        tempo_utils::Status prepareExtension(
            const std::string &name,
            const DataReference &ref,
            CallableInvoker &invoker);

        bool isCompletelyDefined() const;

    private:
        lyric_importer::ImplImport *m_implImport = nullptr;
        ObjectState *m_state;

        ImplHandlePriv *load();
    };
}

#endif // LYRIC_ASSEMBLER_IMPL_HANDLE_H
