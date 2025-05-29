
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/base_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/load_object.h>
#include <lyric_assembler/literal_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_plugin.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/log_stream.h>

#include "lyric_assembler/object_plugin.h"
#include "lyric_runtime/trap_index.h"

lyric_assembler::ObjectState::ObjectState(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    tempo_tracing::ScopeManager *scopeManager,
    const ObjectStateOptions &options)
    : m_location(location),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_scopeManager(scopeManager),
      m_options(options)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_localModuleCache != nullptr);
    TU_ASSERT (m_systemModuleCache != nullptr);
    TU_ASSERT (m_shortcutResolver != nullptr);
    TU_ASSERT (m_scopeManager != nullptr);

    if (!m_options.preludeLocation.isValid()) {
        m_options.preludeLocation = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
    }
}

lyric_assembler::ObjectState::ObjectState(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    tempo_tracing::ScopeManager *scopeManager,
    const lyric_common::ModuleLocation &pluginLocation,
    const ObjectStateOptions &options)
    : ObjectState(
        location,
        std::move(localModuleCache),
        std::move(systemModuleCache),
        std::move(shortcutResolver),
        scopeManager,
        options)
{
    m_pluginLocation = pluginLocation;
}

lyric_assembler::ObjectState::~ObjectState()
{
    delete m_implcache;
    delete m_typecache;
    delete m_literalcache;
    delete m_symbolcache;
    delete m_importcache;
    delete m_fundamentalcache;
    delete m_root;
}

lyric_common::ModuleLocation
lyric_assembler::ObjectState::getLocation() const
{
    return m_location;
}

tempo_tracing::ScopeManager *
lyric_assembler::ObjectState::scopeManager() const
{
    return m_scopeManager;
}

const lyric_assembler::ObjectStateOptions *
lyric_assembler::ObjectState::getOptions() const
{
    return &m_options;
}

/**
 * Initialize the object state. Notably, this process ensures that all fundamental symbols exist,
 * are imported into the symbol cache, and are bound to the environment.
 *
 * @return Status
 */
tempo_utils::Status
lyric_assembler::ObjectState::createRoot(const lyric_common::ModuleLocation &preludeLocation)
{
    if (m_root != nullptr)
        return logAndContinue(AssemblerCondition::kAssemblerInvariant,
            tempo_tracing::LogSeverity::kError,
            "object state is already initialized");

    m_root = new ObjectRoot(this);

    // construct the assembler component classes
    m_tracer = new AssemblerTracer(m_scopeManager);
    m_literalcache = new LiteralCache(m_tracer);
    m_fundamentalcache = new FundamentalCache(preludeLocation, m_tracer);
    m_symbolcache = new SymbolCache(this, m_tracer);
    m_typecache = new TypeCache(this, m_tracer);
    m_implcache = new ImplCache(this, m_tracer);
    m_importcache = new ImportCache(this, m_localModuleCache, m_systemModuleCache,
        m_shortcutResolver, m_symbolcache, m_tracer);

    // load the prelude object
    std::shared_ptr<lyric_importer::ModuleImport> preludeImport;
    TU_ASSIGN_OR_RETURN (preludeImport, m_importcache->importModule(
        preludeLocation, ImportFlags::SystemBootstrap));

    // initialize the root
    TU_RETURN_IF_NOT_OK (m_root->initialize(preludeImport));

    // if specified then find the plugin associated with the module
    if (m_pluginLocation.isValid()) {
        auto localLoader = m_localModuleCache->getLoader();
        auto pluginSpecifier = lyric_object::PluginSpecifier::systemDefault();
        Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>> pluginOption;
        TU_ASSIGN_OR_RETURN (pluginOption, localLoader->loadPlugin(m_pluginLocation, pluginSpecifier));

        if (pluginOption.isEmpty())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "missing plugin {}", m_pluginLocation.toString());

        auto trapIndex = std::make_unique<lyric_runtime::TrapIndex>(pluginOption.getValue());
        TU_RETURN_IF_NOT_OK (trapIndex->initialize());

        m_plugin = std::make_unique<ObjectPlugin>(m_pluginLocation, std::move(trapIndex));
    }

    return {};
}

tempo_utils::Status
lyric_assembler::ObjectState::load()
{
    if (m_root != nullptr)
        return logAndContinue(AssemblerCondition::kAssemblerInvariant,
            tempo_tracing::LogSeverity::kError,
            "object state is already initialized");

    std::shared_ptr<lyric_importer::ModuleImport> moduleImport;

    // import module from the location specified in the object state
    TU_ASSIGN_OR_RETURN (moduleImport, m_localModuleCache->importModule(m_location));
    auto root = moduleImport->getObject().getObject();

    // determine the prelude location from the object
    lyric_common::ModuleLocation preludeLocation;
    TU_ASSIGN_OR_RETURN (preludeLocation, internal::find_system_bootstrap(moduleImport, this));

    // create the object root using the specified prelude location
    TU_RETURN_IF_NOT_OK (createRoot(preludeLocation));

    // copy all symbols into the state
    TU_RETURN_IF_NOT_OK (internal::load_object_symbols(moduleImport, this));

    return {};
}

/**
 * Initialize the object state. Notably, this process ensures that all fundamental symbols exist,
 * are imported into the symbol cache, and are bound to the environment.
 *
 * @return Status
 */
tempo_utils::Result<lyric_assembler::ObjectRoot *>
lyric_assembler::ObjectState::defineRoot()
{
    TU_RETURN_IF_NOT_OK (createRoot(m_options.preludeLocation));
    return m_root;
}

lyric_assembler::ObjectRoot *
lyric_assembler::ObjectState::objectRoot() const
{
    return m_root;
}

lyric_assembler::ObjectPlugin *
lyric_assembler::ObjectState::objectPlugin() const
{
    return m_plugin.get();
}

lyric_assembler::FundamentalCache *
lyric_assembler::ObjectState::fundamentalCache() const
{
    return m_fundamentalcache;
}

lyric_assembler::ImportCache *
lyric_assembler::ObjectState::importCache() const
{
    return m_importcache;
}

lyric_assembler::SymbolCache *
lyric_assembler::ObjectState::symbolCache() const
{
    return m_symbolcache;
}

lyric_assembler::LiteralCache *
lyric_assembler::ObjectState::literalCache() const
{
    return m_literalcache;
}

lyric_assembler::TypeCache *
lyric_assembler::ObjectState::typeCache() const
{
    return m_typecache;
}

lyric_assembler::ImplCache *
lyric_assembler::ObjectState::implCache() const
{
    return m_implcache;
}

tempo_utils::Status
lyric_assembler::ObjectState::appendNamespace(NamespaceSymbol *namespaceSymbol)
{
    TU_ASSERT (namespaceSymbol != nullptr);
    auto symbolUrl = namespaceSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append namespace; symbol {} already exists", symbolUrl.toString());
    m_namespaces.push_back(namespaceSymbol);
    m_symbolcache->insertSymbol(symbolUrl, namespaceSymbol);
    return {};
}

std::vector<lyric_assembler::NamespaceSymbol *>::const_iterator
lyric_assembler::ObjectState::namespacesBegin() const
{
    return m_namespaces.cbegin();
}

std::vector<lyric_assembler::NamespaceSymbol *>::const_iterator
lyric_assembler::ObjectState::namespacesEnd() const
{
    return m_namespaces.cend();
}

int
lyric_assembler::ObjectState::numNamespaces() const
{
    return m_namespaces.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendExistential(ExistentialSymbol *existentialSymbol)
{
    TU_ASSERT (existentialSymbol != nullptr);
    auto symbolUrl = existentialSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append existential; symbol {} already exists", symbolUrl.toString());
    m_existentials.push_back(existentialSymbol);
    m_symbolcache->insertSymbol(symbolUrl, existentialSymbol);
    return {};
}

std::vector<lyric_assembler::ExistentialSymbol *>::const_iterator
lyric_assembler::ObjectState::existentialsBegin() const
{
    return m_existentials.cbegin();
}

std::vector<lyric_assembler::ExistentialSymbol *>::const_iterator
lyric_assembler::ObjectState::existentialsEnd() const
{
    return m_existentials.cend();
}

int
lyric_assembler::ObjectState::numExistentials() const
{
    return m_existentials.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendStatic(StaticSymbol *staticSymbol)
{
    TU_ASSERT (staticSymbol != nullptr);
    auto symbolUrl = staticSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append static; symbol {} already exists", symbolUrl.toString());
    m_statics.push_back(staticSymbol);
    m_symbolcache->insertSymbol(symbolUrl, staticSymbol);
    return {};
}

std::vector<lyric_assembler::StaticSymbol *>::const_iterator
lyric_assembler::ObjectState::staticsBegin() const
{
    return m_statics.cbegin();
}

std::vector<lyric_assembler::StaticSymbol *>::const_iterator
lyric_assembler::ObjectState::staticsEnd() const
{
    return m_statics.cend();
}

int
lyric_assembler::ObjectState::numStatics() const
{
    return m_statics.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendField(FieldSymbol *fieldSymbol)
{
    TU_ASSERT (fieldSymbol != nullptr);
    auto symbolUrl = fieldSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append field; symbol {} already exists", symbolUrl.toString());
    m_fields.push_back(fieldSymbol);
    m_symbolcache->insertSymbol(symbolUrl, fieldSymbol);
    return {};
}

std::vector<lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::ObjectState::fieldsBegin() const
{
    return m_fields.cbegin();
}

std::vector<lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::ObjectState::fieldsEnd() const
{
    return m_fields.cend();
}

int
lyric_assembler::ObjectState::numFields() const
{
    return m_fields.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendAction(ActionSymbol *actionSymbol)
{
    TU_ASSERT (actionSymbol != nullptr);
    auto symbolUrl = actionSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append action; symbol {} already exists", symbolUrl.toString());
    m_actions.push_back(actionSymbol);
    m_symbolcache->insertSymbol(symbolUrl, actionSymbol);
    return {};
}

std::vector<lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::ObjectState::actionsBegin() const
{
    return m_actions.cbegin();
}

std::vector<lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::ObjectState::actionsEnd() const
{
    return m_actions.cend();
}

int
lyric_assembler::ObjectState::numActions() const
{
    return m_actions.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendCall(CallSymbol *callSymbol)
{
    TU_ASSERT (callSymbol != nullptr);
    auto symbolUrl = callSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append call; symbol {} already exists", symbolUrl.toString());
    m_calls.push_back(callSymbol);
    m_symbolcache->insertSymbol(symbolUrl, callSymbol);
    return {};
}

std::vector<lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ObjectState::callsBegin() const
{
    return m_calls.cbegin();
}

std::vector<lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ObjectState::callsEnd() const
{
    return m_calls.cend();
}

int
lyric_assembler::ObjectState::numCalls() const
{
    return m_calls.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendConcept(ConceptSymbol *conceptSymbol)
{
    TU_ASSERT (conceptSymbol != nullptr);
    auto symbolUrl = conceptSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append concept; symbol {} already exists", symbolUrl.toString());
    m_concepts.push_back(conceptSymbol);
    m_symbolcache->insertSymbol(symbolUrl, conceptSymbol);
    return {};
}

std::vector<lyric_assembler::ConceptSymbol *>::const_iterator
lyric_assembler::ObjectState::conceptsBegin() const
{
    return m_concepts.cbegin();
}

std::vector<lyric_assembler::ConceptSymbol *>::const_iterator
lyric_assembler::ObjectState::conceptsEnd() const
{
    return m_concepts.cend();
}

int
lyric_assembler::ObjectState::numConcepts() const
{
    return m_concepts.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendClass(ClassSymbol *classSymbol)
{
    TU_ASSERT (classSymbol != nullptr);
    auto symbolUrl = classSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append class; symbol {} already exists", symbolUrl.toString());
    m_classes.push_back(classSymbol);
    m_symbolcache->insertSymbol(symbolUrl, classSymbol);
    return {};
}

std::vector<lyric_assembler::ClassSymbol *>::const_iterator
lyric_assembler::ObjectState::classesBegin() const
{
    return m_classes.cbegin();
}

std::vector<lyric_assembler::ClassSymbol *>::const_iterator
lyric_assembler::ObjectState::classesEnd() const
{
    return m_classes.cend();
}

int
lyric_assembler::ObjectState::numClasses() const
{
    return m_classes.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendStruct(StructSymbol *structSymbol)
{
    TU_ASSERT (structSymbol != nullptr);
    auto symbolUrl = structSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append struct; symbol {} already exists", symbolUrl.toString());
    m_structs.push_back(structSymbol);
    m_symbolcache->insertSymbol(symbolUrl, structSymbol);
    return {};
}

std::vector<lyric_assembler::StructSymbol *>::const_iterator
lyric_assembler::ObjectState::structsBegin() const
{
    return m_structs.cbegin();
}

std::vector<lyric_assembler::StructSymbol *>::const_iterator
lyric_assembler::ObjectState::structsEnd() const
{
    return m_structs.cend();
}

int
lyric_assembler::ObjectState::numStructs() const
{
    return m_structs.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendInstance(InstanceSymbol *instanceSymbol)
{
    TU_ASSERT (instanceSymbol != nullptr);
    auto symbolUrl = instanceSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append instance; symbol {} already exists", symbolUrl.toString());
    m_instances.push_back(instanceSymbol);
    m_symbolcache->insertSymbol(symbolUrl, instanceSymbol);
    return {};
}

std::vector<lyric_assembler::InstanceSymbol *>::const_iterator
lyric_assembler::ObjectState::instancesBegin() const
{
    return m_instances.cbegin();
}

std::vector<lyric_assembler::InstanceSymbol *>::const_iterator
lyric_assembler::ObjectState::instancesEnd() const
{
    return m_instances.cend();
}

int
lyric_assembler::ObjectState::numInstances() const
{
    return m_instances.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendEnum(EnumSymbol *enumSymbol)
{
    TU_ASSERT (enumSymbol != nullptr);
    auto symbolUrl = enumSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append enum; symbol {} already exists", symbolUrl.toString());
    m_enums.push_back(enumSymbol);
    m_symbolcache->insertSymbol(symbolUrl, enumSymbol);
    return {};
}

std::vector<lyric_assembler::EnumSymbol *>::const_iterator
lyric_assembler::ObjectState::enumsBegin() const
{
    return m_enums.cbegin();
}

std::vector<lyric_assembler::EnumSymbol *>::const_iterator
lyric_assembler::ObjectState::enumsEnd() const
{
    return m_enums.cend();
}

int
lyric_assembler::ObjectState::numEnums() const
{
    return m_enums.size();
}

tempo_utils::Status lyric_assembler::ObjectState::appendBinding(BindingSymbol *bindingSymbol)
{
    TU_ASSERT (bindingSymbol != nullptr);
    auto symbolUrl = bindingSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append binding; symbol {} already exists", symbolUrl.toString());
    m_bindings.push_back(bindingSymbol);
    m_symbolcache->insertSymbol(symbolUrl, bindingSymbol);
    return {};
}

std::vector<lyric_assembler::BindingSymbol *>::const_iterator
lyric_assembler::ObjectState::bindingsBegin() const
{
    return m_bindings.cbegin();
}

std::vector<lyric_assembler::BindingSymbol *>::const_iterator
lyric_assembler::ObjectState::bindingsEnd() const
{
    return m_bindings.cend();
}

int
lyric_assembler::ObjectState::numBindings() const
{
    return m_bindings.size();
}

tempo_utils::Status
lyric_assembler::ObjectState::appendUndeclared(UndeclaredSymbol *undeclaredSymbol)
{
    TU_ASSERT (undeclaredSymbol != nullptr);
    auto symbolUrl = undeclaredSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append undeclared; symbol {} already exists", symbolUrl.toString());
    m_undecls.push_back(undeclaredSymbol);
    m_symbolcache->insertSymbol(symbolUrl, undeclaredSymbol);
    return {};
}

std::vector<lyric_assembler::UndeclaredSymbol *>::const_iterator
lyric_assembler::ObjectState::undeclaredBegin() const
{
    return m_undecls.cbegin();
}

std::vector<lyric_assembler::UndeclaredSymbol *>::const_iterator
lyric_assembler::ObjectState::undeclaredEnd() const
{
    return m_undecls.cend();
}

int
lyric_assembler::ObjectState::numUndeclared() const
{
    return m_undecls.size();
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_assembler::ObjectState::toObject() const
{
    ObjectWriter writer(this);
    TU_RETURN_IF_NOT_OK (writer.initialize());
    return writer.toObject();
}
