
#include <lyric_importer/importer_result.h>
#include <lyric_importer/module_cache.h>

/**
 * Private constructor.
 */
lyric_importer::ModuleCache::ModuleCache(std::shared_ptr<lyric_runtime::AbstractLoader> loader)
    : m_loader(std::move(loader))
{
    m_lock = new absl::Mutex();
}

lyric_importer::ModuleCache::~ModuleCache()
{
    delete m_lock;
}

/**
 * Construct a new shared module cache.
 *
 * @return The thread-safe, shared module cache
 */
std::shared_ptr<lyric_importer::ModuleCache>
lyric_importer::ModuleCache::create(std::shared_ptr<lyric_runtime::AbstractLoader> loader)
{
    return std::shared_ptr<ModuleCache>(new ModuleCache(std::move(loader)));
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_importer::ModuleCache::getLoader() const
{
    return m_loader;
}

/**
 * Returns whether the module cache contains the specified module.
 *
 * @param location The location of the module.
 * @return true if the module cache contains the module, otherwise false.
 */
bool
lyric_importer::ModuleCache::hasModule(const lyric_common::ModuleLocation &location) const
{
    absl::MutexLock locker(m_lock);
    return m_moduleImports.contains(location);
}

/**
 * Returns the thread-safe shared module import if it exists in the cache.
 *
 * @param location The location of the module.
 * @return The shared module import is it exists in the cache, otherwise an empty shared ptr.
 */
std::shared_ptr<lyric_importer::ModuleImport>
lyric_importer::ModuleCache::getModule(const lyric_common::ModuleLocation &location) const
{
    absl::MutexLock locker(m_lock);

    if (m_moduleImports.contains(location))
        return m_moduleImports.at(location);
    return {};
}

// /**
//  *
//  * @param location
//  * @param object
//  * @param mode
//  * @return
//  */
// tempo_utils::Result<std::shared_ptr<lyric_importer::ModuleImport>>
// lyric_importer::ModuleCache::insertModule(
//     const lyric_common::ModuleLocation &location,
//     const lyric_object::LyricObject &object)
// {
//     absl::MutexLock locker(m_lock);
//
//     if (m_moduleImports.contains(location))
//         return m_moduleImports.at(location);
//
//     auto moduleImport = std::shared_ptr<ModuleImport>(new ModuleImport(location, object));
//     TU_RETURN_IF_NOT_OK(moduleImport->initialize());
//
//     m_moduleImports[location] = moduleImport;
//     return moduleImport;
// }

tempo_utils::Result<std::shared_ptr<lyric_importer::ModuleImport>>
lyric_importer::ModuleCache::importModule(const lyric_common::ModuleLocation &location)
{
    absl::MutexLock locker(m_lock);

    lyric_common::ModuleLocation objectLocation;
    if (!location.hasScheme()) {
        Option<lyric_common::ModuleLocation> resolvedOption;
        TU_ASSIGN_OR_RETURN (resolvedOption, m_loader->resolveModule(location));
        if (resolvedOption.isEmpty())
            return ImporterStatus::forCondition(
                ImporterCondition::kModuleNotFound, "{} could not be resolved to an absolute location",
                location.toString());
        objectLocation = resolvedOption.getValue();
    } else {
        objectLocation = location;
    }

    auto entry = m_moduleImports.find(objectLocation);
    if (entry != m_moduleImports.cend())
        return entry->second;

    auto loadModuleResult = m_loader->loadModule(objectLocation);
    TU_RETURN_IF_STATUS(loadModuleResult);
    auto objectOption = loadModuleResult.getResult();
    if (objectOption.isEmpty())
        return ImporterStatus::forCondition(
            ImporterCondition::kModuleNotFound, "module {} not found",
            objectLocation.toString());
    auto object = objectOption.getValue();
    auto root = object.getObject();

    lyric_common::ModuleLocation pluginLocation;
    std::shared_ptr<const lyric_runtime::AbstractPlugin> plugin;
    if (root.hasPlugin()) {
        auto walker = root.getPlugin();

        pluginLocation = walker.getPluginLocation();
        if (pluginLocation.isValid()) {
            pluginLocation = objectLocation.resolve(pluginLocation);
        } else {
            pluginLocation = objectLocation;
        }
        Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>> pluginOption;
        TU_ASSIGN_OR_RETURN (pluginOption,
            m_loader->loadPlugin(pluginLocation, lyric_object::PluginSpecifier::systemDefault()));
        if (pluginOption.isEmpty())
            return ImporterStatus::forCondition(ImporterCondition::kModuleNotFound,
                "plugin {} not found", pluginLocation.toString());
        plugin = pluginOption.getValue();
    }

    auto moduleImport = std::shared_ptr<ModuleImport>(
        new ModuleImport(objectLocation, object, pluginLocation, plugin));
    TU_RETURN_IF_NOT_OK(moduleImport->initialize());

    m_moduleImports[objectLocation] = moduleImport;
    return moduleImport;
}

tempo_utils::Result<lyric_importer::ActionImport *>
lyric_importer::ModuleCache::getAction(const lyric_common::SymbolUrl &actionUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(actionUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(actionUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Action)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not an action",
            actionUrl.toString());

    return moduleImport->getAction(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::BindingImport *>
lyric_importer::ModuleCache::getBinding(const lyric_common::SymbolUrl &bindingUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(bindingUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(bindingUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Binding)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a binding",
            bindingUrl.toString());

    return moduleImport->getBinding(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::CallImport *>
lyric_importer::ModuleCache::getCall(const lyric_common::SymbolUrl &callUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(callUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(callUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Call)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a call",
            callUrl.toString());

    return moduleImport->getCall(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::ClassImport *>
lyric_importer::ModuleCache::getClass(const lyric_common::SymbolUrl &classUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(classUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(classUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Class)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a class",
            classUrl.toString());

    return moduleImport->getClass(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::ConceptImport *>
lyric_importer::ModuleCache::getConcept(const lyric_common::SymbolUrl &conceptUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(conceptUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(conceptUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Concept)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a concept",
            conceptUrl.toString());

    return moduleImport->getConcept(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::EnumImport *>
lyric_importer::ModuleCache::getEnum(const lyric_common::SymbolUrl &enumUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(enumUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(enumUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Enum)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not an enum",
            enumUrl.toString());

    return moduleImport->getEnum(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::ExistentialImport *>
lyric_importer::ModuleCache::getExistential(const lyric_common::SymbolUrl &existentialUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(existentialUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(existentialUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Existential)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not an existential",
            existentialUrl.toString());

    return moduleImport->getExistential(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::FieldImport *>
lyric_importer::ModuleCache::getField(const lyric_common::SymbolUrl &fieldUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(fieldUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(fieldUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Field)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a field",
            fieldUrl.toString());

    return moduleImport->getField(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::InstanceImport *>
lyric_importer::ModuleCache::getInstance(const lyric_common::SymbolUrl &instanceUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(instanceUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(instanceUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Instance)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not an instance",
            instanceUrl.toString());

    return moduleImport->getInstance(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::NamespaceImport *>
lyric_importer::ModuleCache::getNamespace(const lyric_common::SymbolUrl &namespaceUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(namespaceUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(namespaceUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Namespace)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a field",
            namespaceUrl.toString());

    return moduleImport->getNamespace(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::StaticImport *>
lyric_importer::ModuleCache::getStatic(const lyric_common::SymbolUrl &staticUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(staticUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(staticUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Static)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a static",
            staticUrl.toString());

    return moduleImport->getStatic(symbolWalker.getLinkageIndex());
}

tempo_utils::Result<lyric_importer::StructImport *>
lyric_importer::ModuleCache::getStruct(const lyric_common::SymbolUrl &structUrl)
{
    std::shared_ptr<ModuleImport> moduleImport;
    TU_ASSIGN_OR_RETURN(moduleImport, importModule(structUrl.getModuleLocation()));

    auto object = moduleImport->getObject().getObject();
    auto symbolWalker = object.findSymbol(structUrl.getSymbolPath());

    if (symbolWalker.getLinkageSection() != lyric_object::LinkageSection::Struct)
        return ImporterStatus::forCondition(
            ImporterCondition::kImportError, "symbol {} is not a struct",
            structUrl.toString());

    return moduleImport->getStruct(symbolWalker.getLinkageIndex());
}
