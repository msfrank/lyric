#ifndef LYRIC_ASSEMBLER_IMPL_HANDLE_H
#define LYRIC_ASSEMBLER_IMPL_HANDLE_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/impl_import.h>

#include "abstract_callable.h"
#include "assembly_state.h"
#include "base_handle.h"
#include "block_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ImplHandlePriv {
        ImplOffset offset;
        std::string name;
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
            ImplOffset offset,
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            BlockHandle *parentBlock,
            AssemblyState *state);
        ImplHandle(
            ImplOffset offset,
            const std::string &name,
            TypeHandle *implType,
            ConceptSymbol *implConcept,
            const lyric_common::SymbolUrl &receiverUrl,
            TemplateHandle *receiverTemplate,
            BlockHandle *parentBlock,
            AssemblyState *state);

        ImplHandle(lyric_importer::ImplImport *implImport, AssemblyState *state);

        ImplOffset getOffset() const;
        std::string getName() const;

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
        AssemblyState *m_state;

        ImplHandlePriv *load();
    };
}

#endif // LYRIC_ASSEMBLER_IMPL_HANDLE_H
