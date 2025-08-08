
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
#include <lyric_assembler/linkage_symbol.h>
#include <lyric_runtime/trap_index.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::ObjectState::ObjectState(
    const lyric_common::ModuleLocation &location,
    const lyric_common::ModuleLocation &origin,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    const ObjectStateOptions &options)
    : m_location(location),
      m_origin(origin),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_options(options)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_origin.isValid());
    TU_ASSERT (m_localModuleCache != nullptr);
    TU_ASSERT (m_systemModuleCache != nullptr);
    TU_ASSERT (m_shortcutResolver != nullptr);

    if (!m_options.preludeLocation.isValid()) {
        m_options.preludeLocation = lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION);
    }
}

lyric_assembler::ObjectState::ObjectState(
    const lyric_common::ModuleLocation &location,
    const lyric_common::ModuleLocation &origin,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    const lyric_common::ModuleLocation &pluginLocation,
    const ObjectStateOptions &options)
    : ObjectState(
        location,
        origin,
        std::move(localModuleCache),
        std::move(systemModuleCache),
        std::move(shortcutResolver),
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

lyric_common::ModuleLocation
lyric_assembler::ObjectState::getOrigin() const
{
    return m_origin;
}

lyric_common::ModuleLocation
lyric_assembler::ObjectState::getPluginLocation() const
{
    return m_pluginLocation;
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "object state is already initialized");
    if (!m_location.isRelative())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid location {} for object state; location must be relative",
            m_location.toString());
    if (!m_origin.isAbsolute())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid origin {} for object state; origin must be absolute",
            m_origin.toString());

    if (m_pluginLocation.isValid() && !m_pluginLocation.isRelative())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid plugin location {} for object state; plugin location must be relative",
            m_pluginLocation.toString());

    m_root = new ObjectRoot(this);

    // construct the assembler component classes
    m_literalcache = new LiteralCache();
    m_fundamentalcache = new FundamentalCache(preludeLocation);
    m_symbolcache = new SymbolCache(this);
    m_typecache = new TypeCache(this);
    m_implcache = new ImplCache(this);
    m_importcache = new ImportCache(this, m_localModuleCache, m_systemModuleCache,
        m_shortcutResolver, m_symbolcache);

    // initialize the root
    TU_RETURN_IF_NOT_OK (m_root->initialize(preludeLocation, m_options.environmentModules));

    // if specified then find the plugin associated with the module
    if (m_pluginLocation.isValid()) {
        lyric_common::ModuleLocation pluginLocation;
        TU_ASSIGN_OR_RETURN (pluginLocation, m_importcache->resolveImportLocation(m_pluginLocation.toUrl()));

        auto localLoader = m_localModuleCache->getLoader();
        auto pluginSpecifier = lyric_object::PluginSpecifier::systemDefault();
        Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>> pluginOption;
        TU_ASSIGN_OR_RETURN (pluginOption, localLoader->loadPlugin(pluginLocation, pluginSpecifier));

        if (pluginOption.isEmpty())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "missing plugin {}", pluginLocation.toString());

        auto trapIndex = std::make_unique<lyric_runtime::TrapIndex>(pluginOption.getValue());
        TU_RETURN_IF_NOT_OK (trapIndex->initialize());

        m_plugin = std::make_unique<ObjectPlugin>(pluginLocation, std::move(trapIndex));
    }

    return {};
}

tempo_utils::Status
lyric_assembler::ObjectState::load()
{
    if (m_root != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
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

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::ObjectState::appendAction(
    std::unique_ptr<ActionSymbol> &&actionSymbol,
    TypenameSymbol *existingTypename)
{
    if (actionSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid action symbol");
    auto symbolUrl = actionSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, actionSymbol.get(), existingTypename));
    auto *actionPtr = actionSymbol.release();
    m_actions.push_back(actionPtr);
    return actionPtr;
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

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_assembler::ObjectState::appendBinding(
        std::unique_ptr<BindingSymbol> &&bindingSymbol,
        TypenameSymbol *existingTypename)
{
    if (bindingSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid binding symbol");
    auto symbolUrl = bindingSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, bindingSymbol.get(), existingTypename));
    auto *bindingPtr = bindingSymbol.release();
    m_bindings.push_back(bindingPtr);
    return bindingPtr;
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

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ObjectState::appendCall(
    std::unique_ptr<CallSymbol> &&callSymbol,
    TypenameSymbol *existingTypename)
{
    if (callSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
    auto symbolUrl = callSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, callSymbol.get(), existingTypename));
    auto *callPtr = callSymbol.release();
    m_calls.push_back(callPtr);
    return callPtr;
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

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_assembler::ObjectState::appendClass(
    std::unique_ptr<ClassSymbol> &&classSymbol,
    TypenameSymbol *existingTypename)
{
    if (classSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid class symbol");
    auto symbolUrl = classSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, classSymbol.get(), existingTypename));
    auto *classPtr = classSymbol.release();
    m_classes.push_back(classPtr);
    return classPtr;
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

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_assembler::ObjectState::appendConcept(
    std::unique_ptr<ConceptSymbol> &&conceptSymbol,
    TypenameSymbol *existingTypename)
{
    if (conceptSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid concept symbol");
    auto symbolUrl = conceptSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, conceptSymbol.get(), existingTypename));
    auto *conceptPtr = conceptSymbol.release();
    m_concepts.push_back(conceptPtr);
    return conceptPtr;
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

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_assembler::ObjectState::appendEnum(
    std::unique_ptr<EnumSymbol> &&enumSymbol,
    TypenameSymbol *existingTypename)
{
    if (enumSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid enum symbol");
    auto symbolUrl = enumSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, enumSymbol.get(), existingTypename));
    auto *enumPtr = enumSymbol.release();
    m_enums.push_back(enumPtr);
    return enumPtr;
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

tempo_utils::Result<lyric_assembler::ExistentialSymbol *>
lyric_assembler::ObjectState::appendExistential(
    std::unique_ptr<ExistentialSymbol> &&existentialSymbol,
    TypenameSymbol *existingTypename)
{
    if (existentialSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid existential symbol");
    auto symbolUrl = existentialSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, existentialSymbol.get(), existingTypename));
    auto *existentialPtr = existentialSymbol.release();
    m_existentials.push_back(existentialPtr);
    return existentialPtr;
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

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::ObjectState::appendField(
    std::unique_ptr<FieldSymbol> &&fieldSymbol,
    TypenameSymbol *existingTypename)
{
    if (fieldSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid field symbol");
    auto symbolUrl = fieldSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, fieldSymbol.get(), existingTypename));
    auto *fieldPtr = fieldSymbol.release();
    m_fields.push_back(fieldPtr);
    return fieldPtr;
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

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_assembler::ObjectState::appendInstance(
    std::unique_ptr<InstanceSymbol> &&instanceSymbol,
    TypenameSymbol *existingTypename)
{
    if (instanceSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid instance symbol");
    auto symbolUrl = instanceSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, instanceSymbol.get(), existingTypename));
    auto *instancePtr = instanceSymbol.release();
    m_instances.push_back(instancePtr);
    return instancePtr;
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

tempo_utils::Result<lyric_assembler::LinkageSymbol *>
lyric_assembler::ObjectState::appendLinkage(
    std::unique_ptr<LinkageSymbol> &&linkageSymbol,
    TypenameSymbol *existingTypename)
{
    if (linkageSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid linkage symbol");
    auto symbolUrl = linkageSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, linkageSymbol.get(), existingTypename));
    auto *linkagePtr = linkageSymbol.release();
    m_linkages.push_back(linkagePtr);
    return linkagePtr;
}

std::vector<lyric_assembler::LinkageSymbol *>::const_iterator
lyric_assembler::ObjectState::linkagesBegin() const
{
    return m_linkages.cbegin();
}

std::vector<lyric_assembler::LinkageSymbol *>::const_iterator
lyric_assembler::ObjectState::linkagesEnd() const
{
    return m_linkages.cend();
}

int
lyric_assembler::ObjectState::numLinkages() const
{
    return m_linkages.size();
}

tempo_utils::Result<lyric_assembler::NamespaceSymbol *>
lyric_assembler::ObjectState::appendNamespace(
    std::unique_ptr<NamespaceSymbol> &&namespaceSymbol,
    TypenameSymbol *existingTypename)
{
    if (namespaceSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid namespace symbol");
    auto symbolUrl = namespaceSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, namespaceSymbol.get(), existingTypename));
    auto *namespacePtr = namespaceSymbol.release();
    m_namespaces.push_back(namespacePtr);
    return namespacePtr;
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

tempo_utils::Result<lyric_assembler::StaticSymbol *>
lyric_assembler::ObjectState::appendStatic(
    std::unique_ptr<StaticSymbol> &&staticSymbol,
    TypenameSymbol *existingTypename)
{
    if (staticSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid static symbol");
    auto symbolUrl = staticSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, staticSymbol.get(), existingTypename));
    auto *staticPtr = staticSymbol.release();
    m_statics.push_back(staticPtr);
    return staticPtr;
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

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_assembler::ObjectState::appendStruct(
    std::unique_ptr<StructSymbol> &&structSymbol,
    TypenameSymbol *existingTypename)
{
    if (structSymbol == nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid struct symbol");
    auto symbolUrl = structSymbol->getSymbolUrl();
    TU_RETURN_IF_NOT_OK (m_symbolcache->insertSymbol(symbolUrl, structSymbol.get(), existingTypename));
    auto *structPtr = structSymbol.release();
    m_structs.push_back(structPtr);
    return structPtr;
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

tempo_utils::Result<lyric_object::LyricObject>
lyric_assembler::ObjectState::toObject() const
{
    ObjectWriter writer(this);
    TU_RETURN_IF_NOT_OK (writer.initialize());
    return writer.toObject();
}
