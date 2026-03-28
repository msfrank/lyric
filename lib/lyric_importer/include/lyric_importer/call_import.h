#ifndef LYRIC_IMPORTER_CALL_IMPORT_H
#define LYRIC_IMPORTER_CALL_IMPORT_H

#include "base_import.h"
#include "importer_types.h"
#include "module_import.h"

namespace lyric_importer {

    class CallImport : public BaseImport {
    public:
        CallImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 callOffset);

        tu_uint32 getCallOffset() const;

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        bool isHidden();
        lyric_object::CallMode getCallMode();
        bool isAbstract();
        bool isOverride();
        bool isFinal();

        bool hasCallTemplate();
        std::weak_ptr<TemplateImport> getCallTemplate();

        std::weak_ptr<TypeImport> getReturnType();

        lyric_common::SymbolUrl getReceiverUrl();
        lyric_common::SymbolUrl getVirtualUrl();

        Parameter getListParameter(tu_uint8 index);
        std::vector<Parameter>::const_iterator listParametersBegin();
        std::vector<Parameter>::const_iterator listParametersEnd();
        tu_uint8 numListParameters();

        Parameter getNamedParameter(tu_uint8 index);
        std::vector<Parameter>::const_iterator namedParametersBegin();
        std::vector<Parameter>::const_iterator namedParametersEnd();
        tu_uint8 numNamedParameters();

        bool hasRestParameter();
        Parameter getRestParameter();

        lyric_common::SymbolUrl getInitializer(std::string_view name);
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator initializersBegin();
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator initializersEnd();
        tu_uint8 numInitializers();

        std::vector<tu_uint8> getInlineBytecode();

    private:
        tu_uint32 m_callOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            lyric_common::SymbolUrl receiverUrl;
            lyric_common::SymbolUrl virtualUrl;
            bool hasTemplate = false;
            std::weak_ptr<TemplateImport> callTemplate;
            std::weak_ptr<TypeImport> returnType;
            bool isDeclOnly = false;
            bool isHidden = false;
            lyric_object::CallMode callMode = lyric_object::CallMode::Invalid;
            bool isAbstract = false;
            bool isFinal = false;
            std::vector<Parameter> listParameters;
            std::vector<Parameter> namedParameters;
            Option<Parameter> restParameter;
            absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
            std::vector<tu_uint8> inlineBytecode;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_CALL_IMPORT_H
