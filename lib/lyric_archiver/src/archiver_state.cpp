
#include <absl/strings/escaping.h>

#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archiver_state.h>
#include <lyric_archiver/copy_action.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_class.h>
#include <lyric_archiver/copy_concept.h>
#include <lyric_archiver/copy_enum.h>
#include <lyric_archiver/copy_field.h>
#include <lyric_archiver/copy_instance.h>
#include <lyric_archiver/copy_proc.h>
#include <lyric_archiver/copy_struct.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <tempo_security/sha256_hash.h>

lyric_archiver::ArchiverState::ArchiverState(
    std::unique_ptr<lyric_assembler::ObjectState> &&objectState,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    lyric_assembler::ObjectRoot *objectRoot)
    : m_objectState(std::move(objectState)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_objectRoot(objectRoot)
{
    TU_ASSERT (m_objectState != nullptr);
    TU_ASSERT (m_systemModuleCache != nullptr);
    TU_ASSERT (m_objectRoot != nullptr);
}

lyric_assembler::ObjectState *
lyric_archiver::ArchiverState::objectState()
{
    return m_objectState.get();
}

lyric_importer::ModuleCache *
lyric_archiver::ArchiverState::systemModuleCache()
{
    return m_systemModuleCache.get();
}

lyric_assembler::ObjectRoot *
lyric_archiver::ArchiverState::objectRoot()
{
    return m_objectRoot;
}

tempo_utils::Status
lyric_archiver::ArchiverState::insertObject(
    const lyric_common::ModuleLocation &location,
    const lyric_object::LyricObject &object)
{
    if (m_moduleImports.contains(location))
        return {};

    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN (moduleImport, lyric_importer::ModuleImport::create(location, object));
    m_moduleImports[location] = std::move(moduleImport);

    return {};
}

tempo_utils::Status
lyric_archiver::ArchiverState::importObject(const lyric_common::ModuleLocation &location)
{
    if (m_moduleImports.contains(location))
        return {};

    auto *importCache = m_objectState->importCache();
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN (moduleImport, importCache->importModule(
        location, lyric_assembler::ImportFlags::ApiLinkage));
    m_moduleImports[location] = std::move(moduleImport);

    return {};
}

bool
lyric_archiver::ArchiverState::hasImport(const lyric_common::ModuleLocation &location) const
{
    return m_moduleImports.contains(location);
}

tempo_utils::Result<lyric_importer::ActionImport *>
lyric_archiver::ArchiverState::importAction(const lyric_common::SymbolUrl &actionUrl)
{
    auto entry = m_moduleImports.find(actionUrl.getModuleLocation());
    if (entry == m_moduleImports.cend())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing module import {}", actionUrl.getModuleLocation().toString());
    auto &moduleImport = entry->second;
    auto object = moduleImport->getObject().getObject();
    auto symbol = object.findSymbol(actionUrl.getSymbolPath());
    return moduleImport->getAction(symbol.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::CallImport *>
lyric_archiver::ArchiverState::importCall(const lyric_common::SymbolUrl &callUrl)
{
    auto entry = m_moduleImports.find(callUrl.getModuleLocation());
    if (entry == m_moduleImports.cend())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing module import {}", callUrl.getModuleLocation().toString());
    auto &moduleImport = entry->second;
    auto object = moduleImport->getObject().getObject();
    auto symbol = object.findSymbol(callUrl.getSymbolPath());
    return moduleImport->getCall(symbol.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::FieldImport *>
lyric_archiver::ArchiverState::importField(const lyric_common::SymbolUrl &fieldUrl)
{
    auto entry = m_moduleImports.find(fieldUrl.getModuleLocation());
    if (entry == m_moduleImports.cend())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing module import {}", fieldUrl.getModuleLocation().toString());
    auto &moduleImport = entry->second;
    auto object = moduleImport->getObject().getObject();
    auto symbol = object.findSymbol(fieldUrl.getSymbolPath());
    return moduleImport->getField(symbol.getLinkageIndex());
}

tempo_utils::Result<lyric_assembler::AbstractSymbol *>
lyric_archiver::ArchiverState::getSymbol(const lyric_common::SymbolUrl &symbolUrl)
{
    auto entry = m_copiedSymbols.find(symbolUrl);
    if (entry != m_copiedSymbols.cend())
        return entry->second;
    auto *symbolCache = m_objectState->symbolCache();
    auto *symbol = symbolCache->getSymbolOrNull(symbolUrl);
    if (symbol == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing symbol {}", symbolUrl.toString());
    return symbol;
}

tempo_utils::Status
lyric_archiver::ArchiverState::putSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    lyric_assembler::AbstractSymbol *symbol)
{
    if (m_copiedSymbols.contains(symbolUrl))
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", symbolUrl.toString());
    m_copiedSymbols[symbolUrl] = symbol;
    return {};
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_archiver::ArchiverState::archiveSymbol(
    const lyric_common::SymbolUrl &symbolUrl,
    SymbolReferenceSet &symbolReferenceSet)
{
    auto moduleLocation = symbolUrl.getModuleLocation();

    auto *globalNamespace = m_objectRoot->globalNamespace();

    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;
    auto importEntry = m_moduleImports.find(moduleLocation);
    if (importEntry == m_moduleImports.cend()) {
        auto *importCache = m_objectState->importCache();
        TU_ASSIGN_OR_RETURN (moduleImport, importCache->importModule(
            symbolUrl.getModuleLocation(), lyric_assembler::ImportFlags::ApiLinkage));
        m_moduleImports[moduleLocation] = moduleImport;
    } else {
        moduleImport = importEntry->second;
    }

    std::string importHash;
    auto hashEntry = m_importHashes.find(moduleLocation);
    if (hashEntry == m_importHashes.cend()) {
        auto hashBytes = tempo_security::Sha256Hash::hash(moduleLocation.toString());
        importHash = absl::StrCat("$", absl::BytesToHexString(hashBytes));
        m_importHashes[moduleLocation] = importHash;
    } else {
        importHash = hashEntry->second;
    }

    auto object = moduleImport->getObject().getObject();
    auto symbol = object.findSymbol(symbolUrl.getSymbolPath());

    switch (symbol.getLinkageSection()) {

        case lyric_object::LinkageSection::Action: {
            auto *actionImport = moduleImport->getAction(symbol.getLinkageIndex());
            lyric_assembler::ActionSymbol *copiedAction;
            TU_ASSIGN_OR_RETURN (copiedAction, copy_action(
                actionImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedAction->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Call: {
            auto *callImport = moduleImport->getCall(symbol.getLinkageIndex());
            lyric_assembler::CallSymbol *copiedCall;
            TU_ASSIGN_OR_RETURN (copiedCall, copy_call(
                callImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedCall->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Class: {
            auto *classImport = moduleImport->getClass(symbol.getLinkageIndex());
            lyric_assembler::ClassSymbol *copiedClass;
            TU_ASSIGN_OR_RETURN (copiedClass, copy_class(
                classImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedClass->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Concept: {
            auto *conceptImport = moduleImport->getConcept(symbol.getLinkageIndex());
            lyric_assembler::ConceptSymbol *copiedConcept;
            TU_ASSIGN_OR_RETURN (copiedConcept, copy_concept(
                conceptImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedConcept->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Enum: {
            auto *enumImport = moduleImport->getEnum(symbol.getLinkageIndex());
            lyric_assembler::EnumSymbol *copiedEnum;
            TU_ASSIGN_OR_RETURN (copiedEnum, copy_enum(
                enumImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedEnum->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Field: {
            auto *fieldImport = moduleImport->getField(symbol.getLinkageIndex());
            lyric_assembler::FieldSymbol *copiedField;
            TU_ASSIGN_OR_RETURN (copiedField, copy_field(
                fieldImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedField->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Instance: {
            auto *instanceImport = moduleImport->getInstance(symbol.getLinkageIndex());
            lyric_assembler::InstanceSymbol *copiedInstance;
            TU_ASSIGN_OR_RETURN (copiedInstance, copy_instance(
                instanceImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedInstance->getSymbolUrl();
        }

        case lyric_object::LinkageSection::Struct: {
            auto *structImport = moduleImport->getStruct(symbol.getLinkageIndex());
            lyric_assembler::StructSymbol *copiedStruct;
            TU_ASSIGN_OR_RETURN (copiedStruct, copy_struct(
                structImport, importHash, globalNamespace, symbolReferenceSet, *this));
            return copiedStruct->getSymbolUrl();
        }

        default:
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive symbol {}; unsupported symbol",
                symbolUrl.toString());
    }
}

tempo_utils::Status
lyric_archiver::ArchiverState::putPendingProc(
    const lyric_common::SymbolUrl &importUrl,
    const lyric_object::LyricObject &object,
    std::shared_ptr<const lyric_runtime::AbstractPlugin> plugin,
    const lyric_object::ProcHeader &header,
    const lyric_object::BytecodeIterator &code,
    lyric_assembler::ProcHandle *procHandle)
{
    auto pendingProc = std::make_unique<PendingProc>();
    pendingProc->object = object;
    pendingProc->plugin = plugin;
    pendingProc->header = header;
    pendingProc->code = code;
    pendingProc->procHandle = procHandle;
    m_pendingProcs[importUrl] = std::move(pendingProc);
    return {};
}

tempo_utils::Status
lyric_archiver::ArchiverState::copyPendingProcs()
{
    // copy all pending procs
    for (auto it = m_pendingProcs.begin(); it != m_pendingProcs.end(); it++) {
        const auto &importUrl = it->first;
        auto &pendingProc = it->second;
        auto objectLocation = importUrl.getModuleLocation();
        auto pluginLocation = pendingProc->object.getObject().getPlugin().getPluginLocation();
        TU_RETURN_IF_NOT_OK (copy_proc(objectLocation, pendingProc->object, pluginLocation,
            pendingProc->plugin, pendingProc->header, &m_copiedSymbols, pendingProc->code,
            pendingProc->procHandle, m_objectState.get()));
    }
    m_pendingProcs.clear();

    return {};
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_archiver::ArchiverState::declareBinding(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *globalNamespace = m_objectRoot->globalNamespace();
    return globalNamespace->declareBinding(name, access, templateParameters);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_archiver::ArchiverState::toObject() const
{
    return m_objectState->toObject();
}
