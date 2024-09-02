#ifndef LYRIC_ASSEMBLER_CALL_SYMBOL_H
#define LYRIC_ASSEMBLER_CALL_SYMBOL_H

#include <vector>

#include <absl/container/flat_hash_set.h>

#include "abstract_resolver.h"
#include "abstract_symbol.h"
#include "base_symbol.h"
#include "object_state.h"

namespace lyric_assembler {

    struct CallSymbolPriv {
        std::vector<Parameter> listParameters;
        std::vector<Parameter> namedParameters;
        Option<Parameter> restParameter;
        lyric_common::TypeDef returnType;
        lyric_common::SymbolUrl receiverUrl;
        lyric_object::AccessType access;
        lyric_object::CallMode mode;
        bool isNoReturn;
        bool isDeclOnly;
        TypeHandle *callType;
        TemplateHandle *callTemplate;
        BlockHandle *parentBlock;
        std::unique_ptr<ProcHandle> proc;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    };

    /**
     * A CallSymbol comes in a few different varieties. Firstly there are free functions,
     * which are calls not bound to a receiver such as a class or instance. Next there are
     * bound methods, which have an associated receiver. Lastly there are inline calls,
     * which are a subset of free functions with the additional restriction that the proc
     * bytecode must be inlineable with no side effects.
     *
     * Free functions and bound methods can be declared generic by supplying a TemplateHandle.
     */
    class CallSymbol : public BaseSymbol<CallAddress,CallSymbolPriv> {
    public:

        CallSymbol(
            const lyric_common::SymbolUrl &entryUrl,
            const lyric_common::TypeDef &returnType,
            CallAddress address,
            TypeHandle *callType,
            ObjectState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const lyric_common::SymbolUrl &receiverUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TemplateHandle *callTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const lyric_common::SymbolUrl &receiverUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TemplateHandle *callTemplate,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            lyric_importer::CallImport *callImport,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;

        tempo_utils::Result<ProcHandle *> defineCall(
            const ParameterPack &parameterPack,
            const lyric_common::TypeDef &returnType = {});

        lyric_common::TypeDef getReturnType() const;
        lyric_common::SymbolUrl getReceiverUrl() const;
        lyric_object::AccessType getAccessType() const;
        lyric_object::CallMode getMode() const;

        bool isBound() const;
        bool isInline() const;
        bool isCtor() const;
        bool isDeclOnly() const;

        AbstractResolver *callResolver() const;
        TemplateHandle *callTemplate() const;
        const ProcHandle *callProc() const;

        TypeHandle *callType();
        ProcHandle *callProc();

        std::vector<Parameter>::const_iterator listPlacementBegin() const;
        std::vector<Parameter>::const_iterator listPlacementEnd() const;
        std::vector<Parameter>::const_iterator namedPlacementBegin() const;
        std::vector<Parameter>::const_iterator namedPlacementEnd() const;
        const Parameter *restPlacement() const;

        bool hasInitializer(const std::string &name) const;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const;
        void putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer);

        tempo_utils::Status finalizeCall();

    private:
        lyric_common::SymbolUrl m_callUrl;
        lyric_importer::CallImport *m_callImport = nullptr;
        ObjectState *m_state;

        CallSymbolPriv *load() override;
    };

    static inline const CallSymbol *cast_symbol_to_call(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CALL);
        return static_cast<const CallSymbol *>(sym);    // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline CallSymbol *cast_symbol_to_call(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CALL);
        return static_cast<CallSymbol *>(sym);          // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_CALL_SYMBOL_H