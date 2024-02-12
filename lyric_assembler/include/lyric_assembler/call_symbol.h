#ifndef LYRIC_ASSEMBLER_CALL_SYMBOL_H
#define LYRIC_ASSEMBLER_CALL_SYMBOL_H

#include <vector>

#include <absl/container/flat_hash_set.h>

#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"

namespace lyric_assembler {

    struct CallSymbolPriv {
        std::vector<lyric_object::Parameter> parameters;
        Option<lyric_object::Parameter> rest;
        lyric_common::TypeDef returnType;
        lyric_common::SymbolUrl receiverUrl;
        lyric_object::AccessType access;
        lyric_object::CallMode mode;
        bool isNoReturn;
        TypeHandle *callType;
        TemplateHandle *callTemplate;
        std::unique_ptr<ProcHandle> proc;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
        absl::flat_hash_set<lyric_common::TypeDef> exitTypes;
    };

    /**
     * @brief A symbol representing a function or method call.
     *
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
            AssemblyState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            const lyric_common::SymbolUrl &receiverUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TypeHandle *callType,
            TemplateHandle *callTemplate,
            BlockHandle *parentBlock,
            AssemblyState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            const lyric_common::SymbolUrl &receiverUrl,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TypeHandle *callType,
            BlockHandle *parentBlock,
            AssemblyState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TypeHandle *callType,
            TemplateHandle *callTemplate,
            BlockHandle *parentBlock,
            AssemblyState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            const std::vector<lyric_object::Parameter> &parameters,
            const Option<lyric_object::Parameter> &rest,
            const lyric_common::TypeDef &returnType,
            lyric_object::AccessType access,
            CallAddress address,
            lyric_object::CallMode mode,
            TypeHandle *callType,
            BlockHandle *parentBlock,
            AssemblyState *state);

        CallSymbol(
            const lyric_common::SymbolUrl &callUrl,
            lyric_importer::CallImport *callImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        std::vector<lyric_object::Parameter> getParameters() const;
        Option<lyric_object::Parameter> getRest() const;
        lyric_common::TypeDef getReturnType() const;
        lyric_common::SymbolUrl getReceiverUrl() const;
        lyric_object::AccessType getAccessType() const;
        lyric_object::CallMode getMode() const;

        bool isBound() const;
        bool isInline() const;
        bool isCtor() const;

        TypeHandle *callType();
        TemplateHandle *callTemplate();
        ProcHandle *callProc();

        bool hasInitializer(const std::string &name) const;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const;
        void putInitializer(const std::string &name, const lyric_common::SymbolUrl &initializer);

        void putExitType(const lyric_common::TypeDef &exitType);
        absl::flat_hash_set<lyric_common::TypeDef> listExitTypes() const;

        std::vector<lyric_object::Parameter>::const_iterator placementBegin() const;
        std::vector<lyric_object::Parameter>::const_iterator placementEnd() const;

    private:
        lyric_common::SymbolUrl m_callUrl;
        lyric_importer::CallImport *m_callImport = nullptr;
        AssemblyState *m_state;

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