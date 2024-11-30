
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>

lyric_assembler::ImportCache::ImportCache(
    lyric_assembler::ObjectState *state,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    SymbolCache *symbolCache,
    AssemblerTracer *tracer)
    : m_state(state),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_symbolCache(symbolCache),
      m_tracer(tracer)
{
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (m_localModuleCache != nullptr);
    TU_ASSERT (m_systemModuleCache != nullptr);
    TU_ASSERT (m_symbolCache != nullptr);
    TU_ASSERT (m_tracer != nullptr);
}

lyric_assembler::ImportCache::~ImportCache()
{
    for (auto &ptr : m_importcache) {
        delete ptr.second;
    }
}

inline tempo_utils::Status
insert_symbol_into_cache(
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
    const lyric_common::SymbolUrl &symbolUrl,
    lyric_object::SymbolWalker &symbolWalker,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_assembler::ObjectState *state)
{
    if (symbolCache->hasSymbol(symbolUrl))
        return {};

    lyric_assembler::AbstractSymbol *symbolPtr = nullptr;

    switch (symbolWalker.getLinkageSection()) {
        case lyric_object::LinkageSection::Action: {
            auto *actionImport = moduleImport->getAction(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::ActionSymbol(symbolUrl, actionImport, state);
            break;
        }
        case lyric_object::LinkageSection::Call: {
            auto *callImport = moduleImport->getCall(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::CallSymbol(symbolUrl, callImport, state);
            break;
        }
        case lyric_object::LinkageSection::Class: {
            auto *classImport = moduleImport->getClass(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::ClassSymbol(symbolUrl, classImport, state);
            break;
        }
        case lyric_object::LinkageSection::Concept: {
            auto *conceptImport = moduleImport->getConcept(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::ConceptSymbol(symbolUrl, conceptImport, state);
            break;
        }
        case lyric_object::LinkageSection::Enum: {
            auto *enumImport = moduleImport->getEnum(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::EnumSymbol(symbolUrl, enumImport, state);
            break;
        }
        case lyric_object::LinkageSection::Existential: {
            auto *existentialImport = moduleImport->getExistential(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::ExistentialSymbol(symbolUrl, existentialImport, state);
            break;
        }
        case lyric_object::LinkageSection::Field: {
            auto *fieldImport = moduleImport->getField(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::FieldSymbol(symbolUrl, fieldImport, state);
            break;
        }
        case lyric_object::LinkageSection::Instance: {
            auto *instanceImport = moduleImport->getInstance(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::InstanceSymbol(symbolUrl, instanceImport, state);
            break;
        }
        case lyric_object::LinkageSection::Namespace: {
            auto *namespaceImport = moduleImport->getNamespace(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::NamespaceSymbol(symbolUrl, namespaceImport, state);
            break;
        }
        case lyric_object::LinkageSection::Static: {
            auto *staticImport = moduleImport->getStatic(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::StaticSymbol(symbolUrl, staticImport, state);
            break;
        }
        case lyric_object::LinkageSection::Struct: {
            auto *structImport = moduleImport->getStruct(symbolWalker.getLinkageIndex());
            symbolPtr = new lyric_assembler::StructSymbol(symbolUrl, structImport, state);
            break;
        }
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kImportError,
                "error importing module {}; invalid symbol {}",
                symbolUrl.getModuleLocation().toString(), symbolUrl.getSymbolPath().toString());
    }

    auto status = symbolCache->insertSymbol(symbolUrl, symbolPtr);
    if (status.notOk()) {
        delete symbolPtr;
        return status;
    }

    return {};
}

static tempo_utils::Status
import_module_symbols(
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
    const lyric_common::ModuleLocation &location,
    const absl::flat_hash_set<lyric_assembler::ImportRef> &importSymbols,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_assembler::BlockHandle *block,
    bool preload)
{
    auto *state = block->blockState();

    auto object = moduleImport->getObject().getObject();

    for (const auto &importRef : importSymbols) {
        auto symbolPath = importRef.getPath();
        auto symbolName = importRef.getName();
        auto symbolUrl = lyric_common::SymbolUrl(location, symbolPath);

        // it's an error to attempt to import an internal symbol
        if (symbolName.starts_with("$"))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kSyntaxError,
                "error importing symbol {} from {}; cannot import an internal symbol",
                symbolPath.toString(), location.toString());

        // it's an error if a binding already exists for the specified name
        if (block->hasBinding(symbolName))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kImportError,
                "error importing symbol {} from {}; block already contains binding {}",
                symbolPath.toString(), location.toString(), symbolName);

        // find the symbol in the object
        auto symbolWalker = object.findSymbol(symbolPath);
        if (!symbolWalker.isValid())
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kImportError,
                "error importing module {}; missing symbol {}",
                location.toString(), symbolPath.toString());

        // load the symbol into the symbol cache immediately if preload is specified
        if (preload) {
            TU_RETURN_IF_NOT_OK(insert_symbol_into_cache(
                moduleImport, symbolUrl, symbolWalker, symbolCache, state));
        }

        // bind the symbol to the block
        TU_RETURN_IF_STATUS (block->declareAlias(symbolName, symbolUrl));
    }

    return lyric_assembler::AssemblerStatus::ok();
}

static tempo_utils::Status
import_module(
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
    const lyric_common::ModuleLocation &location,
    lyric_assembler::SymbolCache *symbolCache,
    lyric_assembler::BlockHandle *block,
    bool preload)
{
    auto *state = block->blockState();

    auto object = moduleImport->getObject().getObject();

    for (int i = 0; i < object.numSymbols(); i++) {
        auto symbolWalker = object.getSymbol(i);
        auto symbolPath = symbolWalker.getSymbolPath();
        auto symbolUrl = lyric_common::SymbolUrl(location, symbolPath);

        // ignore symbols which are inside a namespace
        if (symbolPath.getPath().size() > 1)
            continue;

        // ignore internal symbols (those that start with $)
        auto symbolName = symbolPath.getName();
        if (symbolName.starts_with("$"))
            continue;

        // it's an error if a binding already exists for the specified name
        if (block->hasBinding(symbolName))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kImportError,
                "error importing symbol {} from {}; block already contains binding {}",
                symbolPath.toString(), moduleImport->getLocation().toString(), symbolName);

        // load the symbol into the symbol cache immediately if preload is specified
        if (preload) {
            TU_RETURN_IF_NOT_OK(insert_symbol_into_cache(
                moduleImport, symbolUrl, symbolWalker, symbolCache, state));
        }

        // bind the symbol to the block
        TU_RETURN_IF_STATUS (block->declareAlias(symbolName, symbolUrl));
    }

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Result<std::shared_ptr<lyric_importer::ModuleImport>>
lyric_assembler::ImportCache::importModule(
    const lyric_common::ModuleLocation &importLocation,
    ImportFlags importFlags)
{
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;

    // get the shared module import
    if (!importLocation.hasScheme() && !importLocation.hasAuthority()) {
        TU_ASSIGN_OR_RETURN (moduleImport, m_localModuleCache->importModule(importLocation));
    } else {
        TU_ASSIGN_OR_RETURN (moduleImport, m_systemModuleCache->importModule(importLocation));
    }

    auto location = moduleImport->getLocation();

    // insert import handle into the cache if one doesn't exist
    if (!m_importcache.contains(location)) {
        TU_RETURN_IF_NOT_OK (insertImport(location, importFlags));
    }

    return moduleImport;
}

/**
 *
 * @param importLocation
 * @param block
 * @param importSymbols
 * @return
 */
tempo_utils::Status
lyric_assembler::ImportCache::importModule(
    const lyric_common::ModuleLocation &importLocation,
    BlockHandle *block,
    const absl::flat_hash_set<ImportRef> &importSymbols,
    bool preload)
{
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;

    // get the shared module import
    if (!importLocation.hasScheme() && !importLocation.hasAuthority()) {
        TU_ASSIGN_OR_RETURN (moduleImport, m_localModuleCache->importModule(importLocation));
    } else {
        TU_ASSIGN_OR_RETURN (moduleImport, m_systemModuleCache->importModule(importLocation));
    }
    auto location = moduleImport->getLocation();

    if (!importSymbols.empty()) {
        // if import symbols is not empty, then import only the specified symbols
        TU_RETURN_IF_NOT_OK (import_module_symbols(
            moduleImport, location, importSymbols, m_symbolCache, block, preload));
    } else {
        // otherwise import all symbols which are not inside a namespace
        TU_RETURN_IF_NOT_OK (import_module(moduleImport, location, m_symbolCache, block, preload));
    }

    // insert import handle into the cache if one doesn't exist
    if (!m_importcache.contains(location)) {
        TU_RETURN_IF_NOT_OK (insertImport(location, ImportFlags::ApiLinkage));
    }

    return AssemblerStatus::ok();
}

std::shared_ptr<lyric_importer::ModuleImport>
lyric_assembler::ImportCache::getModule(const lyric_common::ModuleLocation &importLocation)
{
    auto entry = m_importcache.find(importLocation);
    if (entry == m_importcache.cend())
        return {};

    if (entry->second->isShared) {
        return m_systemModuleCache->getModule(importLocation);
    } else {
        return m_localModuleCache->getModule(importLocation);
    }
}

tempo_utils::Result<lyric_assembler::AbstractSymbol *>
lyric_assembler::ImportCache::importSymbol(const lyric_common::SymbolUrl &symbolUrl)
{
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;

    // get the shared module import
    auto importLocation = symbolUrl.getModuleLocation();
    if (!importLocation.hasScheme() && !importLocation.hasAuthority()) {
        TU_ASSIGN_OR_RETURN (moduleImport, m_localModuleCache->importModule(importLocation));
    } else {
        TU_ASSIGN_OR_RETURN (moduleImport, m_systemModuleCache->importModule(importLocation));
    }
    auto location = moduleImport->getLocation();

    auto object = moduleImport->getObject().getObject();
    auto symbolPath = symbolUrl.getSymbolPath();
    auto symbolWalker = object.findSymbol(symbolPath);
    if (!symbolWalker.isValid())
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; missing symbol {}",
            location.toString(), symbolPath.toString());

    // load the symbol into the symbol cache
    TU_RETURN_IF_NOT_OK(insert_symbol_into_cache(moduleImport, symbolUrl, symbolWalker, m_symbolCache, m_state));

    // insert import handle into the cache if one doesn't exist
    if (!m_importcache.contains(location)) {
        TU_RETURN_IF_NOT_OK (insertImport(location, ImportFlags::ApiLinkage));
    }

    // return the symbol
    auto *symbol = m_symbolCache->getSymbolOrNull(symbolUrl);
    TU_ASSERT (symbol != nullptr);
    return symbol;
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::ImportCache::importAction(const lyric_common::SymbolUrl &actionUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(actionUrl));
    if (symbol->getSymbolType() != SymbolType::ACTION)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not an action", actionUrl.toString());
    return cast_symbol_to_action(symbol);
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ImportCache::importCall(const lyric_common::SymbolUrl &callUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(callUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a call", callUrl.toString());
    return cast_symbol_to_call(symbol);
}

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_assembler::ImportCache::importClass(const lyric_common::SymbolUrl &classUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(classUrl));
    if (symbol->getSymbolType() != SymbolType::CLASS)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a class", classUrl.toString());
    return cast_symbol_to_class(symbol);
}

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_assembler::ImportCache::importConcept(const lyric_common::SymbolUrl &conceptUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(conceptUrl));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a concept", conceptUrl.toString());
    return cast_symbol_to_concept(symbol);
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_assembler::ImportCache::importEnum(const lyric_common::SymbolUrl &enumUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(enumUrl));
    if (symbol->getSymbolType() != SymbolType::ENUM)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not an enum", enumUrl.toString());
    return cast_symbol_to_enum(symbol);
}

tempo_utils::Result<lyric_assembler::ExistentialSymbol *>
lyric_assembler::ImportCache::importExistential(const lyric_common::SymbolUrl &existentialUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(existentialUrl));
    if (symbol->getSymbolType() != SymbolType::EXISTENTIAL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not an existential", existentialUrl.toString());
    return cast_symbol_to_existential(symbol);
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::ImportCache::importField(const lyric_common::SymbolUrl &fieldUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(fieldUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a field", fieldUrl.toString());
    return cast_symbol_to_field(symbol);
}

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_assembler::ImportCache::importInstance(const lyric_common::SymbolUrl &instanceUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(instanceUrl));
    if (symbol->getSymbolType() != SymbolType::INSTANCE)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not an instance", instanceUrl.toString());
    return cast_symbol_to_instance(symbol);
}

tempo_utils::Result<lyric_assembler::NamespaceSymbol *>
lyric_assembler::ImportCache::importNamespace(const lyric_common::SymbolUrl &namespaceUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(namespaceUrl));
    if (symbol->getSymbolType() != SymbolType::NAMESPACE)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a namespace", namespaceUrl.toString());
    return cast_symbol_to_namespace(symbol);
}

tempo_utils::Result<lyric_assembler::StaticSymbol *>
lyric_assembler::ImportCache::importStatic(const lyric_common::SymbolUrl &staticUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(staticUrl));
    if (symbol->getSymbolType() != SymbolType::STATIC)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a static", staticUrl.toString());
    return cast_symbol_to_static(symbol);
}

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_assembler::ImportCache::importStruct(const lyric_common::SymbolUrl &structUrl)
{
    AbstractSymbol *symbol = nullptr;
    TU_ASSIGN_OR_RETURN (symbol, importSymbol(structUrl));
    if (symbol->getSymbolType() != SymbolType::STRUCT)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kImportError,
            "error importing symbol {}; symbol is not a struct", structUrl.toString());
    return cast_symbol_to_struct(symbol);
}

bool
lyric_assembler::ImportCache::hasImport(const lyric_common::ModuleLocation &importLocation) const
{
    return m_importcache.contains(importLocation);
}

lyric_assembler::ImportHandle *
lyric_assembler::ImportCache::getImport(const lyric_common::ModuleLocation &importLocation) const
{
    if (!m_importcache.contains(importLocation))
        return nullptr;
    return m_importcache.at(importLocation);
}

tempo_utils::Status
lyric_assembler::ImportCache::insertImport(
    const lyric_common::ModuleLocation &importLocation,
    ImportFlags importFlags)
{
    if (m_importcache.contains(importLocation))
        return m_tracer->logAndContinue(AssemblerCondition::kImportError,
            tempo_tracing::LogSeverity::kError,
            "module {} is already imported", importLocation.toString());
    bool isPrivate = !importLocation.hasScheme() && !importLocation.hasAuthority();
    auto *importHandle = new ImportHandle{importLocation, importFlags, !isPrivate};
    m_importcache[importLocation] = importHandle;
    return AssemblerStatus::ok();
}

absl::flat_hash_map<lyric_common::ModuleLocation,lyric_assembler::ImportHandle *>::const_iterator
lyric_assembler::ImportCache::importsBegin() const
{
    return m_importcache.cbegin();
}

absl::flat_hash_map<lyric_common::ModuleLocation,lyric_assembler::ImportHandle *>::const_iterator
lyric_assembler::ImportCache::importsEnd() const
{
    return m_importcache.cend();
}

int
lyric_assembler::ImportCache::numImports() const
{
    return m_importcache.size();
}