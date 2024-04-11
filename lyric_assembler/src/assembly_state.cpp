
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/assembler_types.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/base_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_object.h>
#include <lyric_assembler/literal_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::AssemblyState::AssemblyState(
    const lyric_common::AssemblyLocation &location,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    tempo_tracing::ScopeManager *scopeManager,
    const AssemblyStateOptions &options)
    : m_location(location),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_scopeManager(scopeManager),
      m_options(options)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_systemModuleCache != nullptr);
    TU_ASSERT (m_scopeManager != nullptr);
    TU_ASSERT (m_options.preludeLocation.isValid());

}

lyric_assembler::AssemblyState::~AssemblyState()
{
    delete m_implcache;
    delete m_typecache;
    delete m_literalcache;
    delete m_symbolcache;
    delete m_importcache;
    delete m_fundamentalcache;
}

lyric_common::AssemblyLocation
lyric_assembler::AssemblyState::getLocation() const
{
    return m_location;
}

tempo_tracing::ScopeManager *
lyric_assembler::AssemblyState::scopeManager() const
{
    return m_scopeManager;
}

const lyric_assembler::AssemblyStateOptions *
lyric_assembler::AssemblyState::getOptions() const
{
    return &m_options;
}

/**
 * Initialize the assembler state. Notably, this process ensures that all fundamental symbols exist,
 * are imported into the symbol cache, and are bound to the environment.
 *
 * @return Status
 */
tempo_utils::Status
lyric_assembler::AssemblyState::initialize()
{
    if (m_tracer != nullptr)
        return logAndContinue(AssemblerCondition::kAssemblerInvariant,
            tempo_tracing::LogSeverity::kError,
            "assembler state is already initialized");

    // construct the assembler component classes
    m_tracer = new AssemblerTracer(m_scopeManager);
    m_literalcache = new LiteralCache(m_tracer);
    m_symbolcache = new SymbolCache(m_tracer);
    m_fundamentalcache = new FundamentalCache(m_options.preludeLocation, m_tracer);
    m_importcache = new ImportCache(this, m_options.workspaceLoader, m_systemModuleCache, m_symbolcache, m_tracer);
    m_typecache = new TypeCache(this, m_tracer);
    m_implcache = new ImplCache(this, m_tracer);

    // load the prelude object
    std::shared_ptr<lyric_importer::ModuleImport> preludeImport;
    TU_ASSIGN_OR_RETURN (preludeImport, m_systemModuleCache->importModule(m_options.preludeLocation));
    auto preludeObject = preludeImport->getObject().getObject();

    // add the prelude to the import cache
    TU_RETURN_IF_NOT_OK (m_importcache->insertImport(m_options.preludeLocation, ImportFlags::SystemBootstrap));

    // import all symbols (fundamental or not) from the prelude into the symbol cache
    for (int i = 0; i < preludeObject.numSymbols(); i++) {
        auto symbolWalker = preludeObject.getSymbol(i);
        lyric_common::SymbolUrl symbolUrl(m_options.preludeLocation, symbolWalker.getSymbolPath());
        TU_RETURN_IF_STATUS (m_importcache->importSymbol(symbolUrl));
    }

    // bind all fundamental symbols from the prelude into the environment
    for (int i = 0; i < static_cast<int>(FundamentalSymbol::NUM_SYMBOLS); i++) {
        auto fundamental = static_cast<lyric_assembler::FundamentalSymbol>(i);
        auto symbolUrl = m_fundamentalcache->getFundamentalUrl(fundamental);
        TU_ASSERT (symbolUrl.isValid());

        auto *sym = m_symbolcache->getSymbol(symbolUrl);
        TU_ASSERT (sym != nullptr);

        auto symbolName = symbolUrl.getSymbolPath().getName();
        if (m_symbolcache->hasEnvBinding(symbolName))
            return logAndContinue(AssemblerCondition::kImportError,
                tempo_tracing::LogSeverity::kError,
                "environment already contains binding {} referencing symbol {}",
                symbolName, m_symbolcache->getEnvBinding(symbolName).symbolUrl.toString());

        switch (sym->getSymbolType()) {

            case lyric_assembler::SymbolType::EXISTENTIAL:
            case lyric_assembler::SymbolType::CONSTANT:
            case lyric_assembler::SymbolType::ACTION:
            case lyric_assembler::SymbolType::CALL:
            case lyric_assembler::SymbolType::CLASS:
            case lyric_assembler::SymbolType::CONCEPT:
            case lyric_assembler::SymbolType::STRUCT:
            case lyric_assembler::SymbolType::ENUM:
            {
                lyric_assembler::SymbolBinding binding;
                binding.symbolUrl = symbolUrl;
                binding.typeDef = sym->getAssignableType();
                binding.bindingType = BindingType::Descriptor;
                m_symbolcache->insertEnvBinding(symbolName, binding);
                break;
            }

            case lyric_assembler::SymbolType::STATIC:
            {
                lyric_assembler::SymbolBinding binding;
                binding.symbolUrl = symbolUrl;
                binding.typeDef = sym->getAssignableType();
                binding.bindingType = cast_symbol_to_static(sym)->isVariable()?
                    BindingType::Variable : BindingType::Value;
                m_symbolcache->insertEnvBinding(symbolName, binding);
                break;
            }

            case lyric_assembler::SymbolType::INSTANCE:
            {
                lyric_assembler::SymbolBinding binding;
                binding.symbolUrl = symbolUrl;
                binding.typeDef = sym->getAssignableType();
                binding.bindingType = BindingType::Descriptor;
                m_symbolcache->insertEnvBinding(symbolName, binding);
                auto *instanceSymbol = cast_symbol_to_instance(sym);
                for (auto iterator = instanceSymbol->implsBegin(); iterator != instanceSymbol->implsEnd(); iterator++) {
                    auto *implHandle = iterator->second;
                    auto implType = implHandle->implType()->getTypeDef();
                    if (m_symbolcache->hasEnvInstance(implType))
                        return logAndContinue(AssemblerCondition::kImportError,
                            tempo_tracing::LogSeverity::kError,
                            "environment already contains instance {}", implType.toString());
                    m_symbolcache->insertEnvInstance(implType, symbolUrl);
                }
                break;
            }

            default:
                break;
        }
    }

    return AssemblerStatus::ok();
}

lyric_assembler::FundamentalCache *
lyric_assembler::AssemblyState::fundamentalCache() const
{
    return m_fundamentalcache;
}

lyric_assembler::ImportCache *
lyric_assembler::AssemblyState::importCache() const
{
    return m_importcache;
}

lyric_assembler::SymbolCache *
lyric_assembler::AssemblyState::symbolCache() const
{
    return m_symbolcache;
}

lyric_assembler::LiteralCache *
lyric_assembler::AssemblyState::literalCache() const
{
    return m_literalcache;
}

lyric_assembler::TypeCache *
lyric_assembler::AssemblyState::typeCache() const
{
    return m_typecache;
}

lyric_assembler::ImplCache *
lyric_assembler::AssemblyState::implCache() const
{
    return m_implcache;
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendNamespace(NamespaceSymbol *namespaceSymbol)
{
    TU_ASSERT (namespaceSymbol != nullptr);
    auto symbolUrl = namespaceSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append namespace; symbol {} already exists", symbolUrl.toString());
    m_namespaces.push_back(namespaceSymbol);
    m_symbolcache->insertSymbol(symbolUrl, namespaceSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchNamespace(NamespaceSymbol *namespaceSymbol)
{
    TU_ASSERT (namespaceSymbol != nullptr);

    // namespace has been touched already, nothing to do
    if (namespaceSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the namespace is in the symbol cache
    auto namespaceUrl = namespaceSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(namespaceUrl))
        throwAssemblerInvariant("missing namespace symbol {}", namespaceUrl.toString());

    //
    auto location = namespaceUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(namespaceSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(namespaceSymbol->getAssignableType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::NamespaceSymbol *>::const_iterator
lyric_assembler::AssemblyState::namespacesBegin() const
{
    return m_namespaces.cbegin();
}

std::vector<lyric_assembler::NamespaceSymbol *>::const_iterator
lyric_assembler::AssemblyState::namespacesEnd() const
{
    return m_namespaces.cend();
}

int
lyric_assembler::AssemblyState::numNamespaces() const
{
    return m_namespaces.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendExistential(ExistentialSymbol *existentialSymbol)
{
    TU_ASSERT (existentialSymbol != nullptr);
    auto symbolUrl = existentialSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append existential; symbol {} already exists", symbolUrl.toString());
    m_existentials.push_back(existentialSymbol);
    m_symbolcache->insertSymbol(symbolUrl, existentialSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchExistential(ExistentialSymbol *existentialSymbol)
{
    TU_ASSERT (existentialSymbol != nullptr);

    // existential has been touched already, nothing to do
    if (existentialSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the existential is in the symbol cache
    auto existentialUrl = existentialSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(existentialUrl))
        throwAssemblerInvariant("missing existential symbol {}", existentialUrl.toString());

    //
    auto location = existentialUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(existentialSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(existentialSymbol->getAssignableType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::ExistentialSymbol *>::const_iterator
lyric_assembler::AssemblyState::existentialsBegin() const
{
    return m_existentials.cbegin();
}

std::vector<lyric_assembler::ExistentialSymbol *>::const_iterator
lyric_assembler::AssemblyState::existentialsEnd() const
{
    return m_existentials.cend();
}

int
lyric_assembler::AssemblyState::numExistentials() const
{
    return m_existentials.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendStatic(StaticSymbol *staticSymbol)
{
    TU_ASSERT (staticSymbol != nullptr);
    auto symbolUrl = staticSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append static; symbol {} already exists", symbolUrl.toString());
    m_statics.push_back(staticSymbol);
    m_symbolcache->insertSymbol(symbolUrl, staticSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchStatic(StaticSymbol *staticSymbol)
{
    TU_ASSERT (staticSymbol != nullptr);

    // static has been touched already, nothing to do
    if (staticSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the static is in the symbol cache
    auto staticUrl = staticSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(staticUrl))
        throwAssemblerInvariant("missing static symbol {}", staticUrl.toString());

    //
    auto location = staticUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(staticSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(staticSymbol->getAssignableType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::StaticSymbol *>::const_iterator
lyric_assembler::AssemblyState::staticsBegin() const
{
    return m_statics.cbegin();
}

std::vector<lyric_assembler::StaticSymbol *>::const_iterator
lyric_assembler::AssemblyState::staticsEnd() const
{
    return m_statics.cend();
}

int
lyric_assembler::AssemblyState::numStatics() const
{
    return m_statics.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendField(FieldSymbol *fieldSymbol)
{
    TU_ASSERT (fieldSymbol != nullptr);
    auto symbolUrl = fieldSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append field; symbol {} already exists", symbolUrl.toString());
    m_fields.push_back(fieldSymbol);
    m_symbolcache->insertSymbol(symbolUrl, fieldSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchField(FieldSymbol *fieldSymbol)
{
    TU_ASSERT (fieldSymbol != nullptr);

    // field has been touched already, nothing to do
    if (fieldSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the field is in the symbol cache
    auto fieldUrl = fieldSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(fieldUrl))
        throwAssemblerInvariant("missing field symbol {}", fieldUrl.toString());

    //
    auto location = fieldUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(fieldSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(fieldSymbol->getAssignableType());
    if (status.notOk())
        return status;

    //
    auto initializerUrl = fieldSymbol->getInitializer();
    if (initializerUrl.isValid()) {
        if (!m_symbolcache->hasSymbol(initializerUrl))
            throwAssemblerInvariant("no initializer found for field symbol {}", fieldUrl.toString());
        status = m_symbolcache->touchSymbol(initializerUrl);
        if (status.notOk())
            return status;
    }

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::AssemblyState::fieldsBegin() const
{
    return m_fields.cbegin();
}

std::vector<lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::AssemblyState::fieldsEnd() const
{
    return m_fields.cend();
}

int
lyric_assembler::AssemblyState::numFields() const
{
    return m_fields.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendAction(ActionSymbol *actionSymbol)
{
    TU_ASSERT (actionSymbol != nullptr);
    auto symbolUrl = actionSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append action; symbol {} already exists", symbolUrl.toString());
    m_actions.push_back(actionSymbol);
    m_symbolcache->insertSymbol(symbolUrl, actionSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchAction(ActionSymbol *actionSymbol)
{
    TU_ASSERT (actionSymbol != nullptr);

    // action has been touched already, nothing to do
    if (actionSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the action is in the symbol cache
    auto actionUrl = actionSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(actionUrl))
        throwAssemblerInvariant("missing action symbol {}", actionUrl.toString());

    //
    auto receiverUrl = actionSymbol->getReceiverUrl();
    if (!receiverUrl.isValid())
        throwAssemblerInvariant("no receiver found for action symbol {}", actionUrl.toString());
    if (!m_symbolcache->hasSymbol(receiverUrl))
        throwAssemblerInvariant("missing receiver {}", receiverUrl.toString());

    //
    auto location = actionUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(actionSymbol);
    if (status.notOk())
        return status;

    status = m_symbolcache->touchSymbol(receiverUrl);
    if (status.notOk())
        return status;

    for (const auto &p : actionSymbol->getParameters()) {
        status = m_typecache->touchType(p.typeDef);
        if (status.notOk())
            return status;
    }

    auto restOption = actionSymbol->getRest();
    if (!restOption.isEmpty()) {
        status = m_typecache->touchType(restOption.getValue().typeDef);
        if (status.notOk())
            return status;
    }

    status = m_typecache->touchType(actionSymbol->getReturnType());
    if (status.notOk())
        return status;

    auto *actionTemplate = actionSymbol->actionTemplate();
    if (actionTemplate) {
        actionTemplate->touch();
    }

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::AssemblyState::actionsBegin() const
{
    return m_actions.cbegin();
}

std::vector<lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::AssemblyState::actionsEnd() const
{
    return m_actions.cend();
}

int
lyric_assembler::AssemblyState::numActions() const
{
    return m_actions.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendCall(CallSymbol *callSymbol)
{
    TU_ASSERT (callSymbol != nullptr);
    auto symbolUrl = callSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append call; symbol {} already exists", symbolUrl.toString());
    m_calls.push_back(callSymbol);
    m_symbolcache->insertSymbol(symbolUrl, callSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchCall(CallSymbol *callSymbol)
{
    TU_ASSERT (callSymbol != nullptr);

    // call has been touched already, nothing to do
    if (callSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the call is in the symbol cache
    auto callUrl = callSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(callUrl))
        throwAssemblerInvariant("missing call symbol {}", callUrl.toString());

    //
    auto receiverUrl = callSymbol->getReceiverUrl();
    if (receiverUrl.isValid()) {
        if (!m_symbolcache->hasSymbol(receiverUrl))
            throwAssemblerInvariant("missing receiver {}", receiverUrl.toString());
    }

    //
    auto location = callUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(callSymbol);
    if (status.notOk())
        return status;

    if (receiverUrl.isValid()) {
        status = m_symbolcache->touchSymbol(receiverUrl);
        if (status.notOk())
            return status;
    }

    status = m_typecache->touchType(callSymbol->callType());
    if (status.notOk())
        return status;

    for (const auto &p : callSymbol->getParameters()) {
        status = m_typecache->touchType(p.typeDef);
        if (status.notOk())
            return status;
    }

    auto restOption = callSymbol->getRest();
    if (!restOption.isEmpty()) {
        status = m_typecache->touchType(restOption.getValue().typeDef);
        if (status.notOk())
            return status;
    }

    status = m_typecache->touchType(callSymbol->getReturnType());
    if (status.notOk())
        return status;

    auto *callTemplate = callSymbol->callTemplate();
    if (callTemplate) {
        callTemplate->touch();
    }

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::AssemblyState::callsBegin() const
{
    return m_calls.cbegin();
}

std::vector<lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::AssemblyState::callsEnd() const
{
    return m_calls.cend();
}

int
lyric_assembler::AssemblyState::numCalls() const
{
    return m_calls.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendConcept(ConceptSymbol *conceptSymbol)
{
    TU_ASSERT (conceptSymbol != nullptr);
    auto symbolUrl = conceptSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append concept; symbol {} already exists", symbolUrl.toString());
    m_concepts.push_back(conceptSymbol);
    m_symbolcache->insertSymbol(symbolUrl, conceptSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchConcept(ConceptSymbol *conceptSymbol)
{
    TU_ASSERT (conceptSymbol != nullptr);

    // concept has been touched already, nothing to do
    if (conceptSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the concept is in the symbol cache
    auto conceptUrl = conceptSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(conceptUrl))
        throwAssemblerInvariant("missing concept symbol {}", conceptUrl.toString());

    //
    auto location = conceptUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(conceptSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(conceptSymbol->conceptType());
    if (status.notOk())
        return status;

    auto *conceptTemplate = conceptSymbol->conceptTemplate();
    if (conceptTemplate) {
        conceptTemplate->touch();
    }

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::ConceptSymbol *>::const_iterator
lyric_assembler::AssemblyState::conceptsBegin() const
{
    return m_concepts.cbegin();
}

std::vector<lyric_assembler::ConceptSymbol *>::const_iterator
lyric_assembler::AssemblyState::conceptsEnd() const
{
    return m_concepts.cend();
}

int
lyric_assembler::AssemblyState::numConcepts() const
{
    return m_concepts.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendClass(ClassSymbol *classSymbol)
{
    TU_ASSERT (classSymbol != nullptr);
    auto symbolUrl = classSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append class; symbol {} already exists", symbolUrl.toString());
    m_classes.push_back(classSymbol);
    m_symbolcache->insertSymbol(symbolUrl, classSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchClass(ClassSymbol *classSymbol)
{
    TU_ASSERT (classSymbol != nullptr);

    // class has been touched already, nothing to do
    if (classSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the class is in the symbol cache
    auto classUrl = classSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(classUrl))
        throwAssemblerInvariant("missing class symbol {}", classUrl.toString());

    //
    auto location = classUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(classSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(classSymbol->classType());
    if (status.notOk())
        return status;

    auto *classTemplate = classSymbol->classTemplate();
    if (classTemplate) {
        classTemplate->touch();
    }

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::ClassSymbol *>::const_iterator
lyric_assembler::AssemblyState::classesBegin() const
{
    return m_classes.cbegin();
}

std::vector<lyric_assembler::ClassSymbol *>::const_iterator
lyric_assembler::AssemblyState::classesEnd() const
{
    return m_classes.cend();
}

int
lyric_assembler::AssemblyState::numClasses() const
{
    return m_classes.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendStruct(StructSymbol *structSymbol)
{
    TU_ASSERT (structSymbol != nullptr);
    auto symbolUrl = structSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append struct; symbol {} already exists", symbolUrl.toString());
    m_structs.push_back(structSymbol);
    m_symbolcache->insertSymbol(symbolUrl, structSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchStruct(StructSymbol *structSymbol)
{
    TU_ASSERT (structSymbol != nullptr);

    // struct has been touched already, nothing to do
    if (structSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the class is in the symbol cache
    auto structUrl = structSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(structUrl))
        throwAssemblerInvariant("missing struct symbol {}", structUrl.toString());

    //
    auto location = structUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(structSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(structSymbol->structType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::StructSymbol *>::const_iterator
lyric_assembler::AssemblyState::structsBegin() const
{
    return m_structs.cbegin();
}

std::vector<lyric_assembler::StructSymbol *>::const_iterator
lyric_assembler::AssemblyState::structsEnd() const
{
    return m_structs.cend();
}

int
lyric_assembler::AssemblyState::numStructs() const
{
    return m_structs.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendInstance(InstanceSymbol *instanceSymbol)
{
    TU_ASSERT (instanceSymbol != nullptr);
    auto symbolUrl = instanceSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append instance; symbol {} already exists", symbolUrl.toString());
    m_instances.push_back(instanceSymbol);
    m_symbolcache->insertSymbol(symbolUrl, instanceSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchInstance(InstanceSymbol *instanceSymbol)
{
    TU_ASSERT (instanceSymbol != nullptr);

    // instance has been touched already, nothing to do
    if (instanceSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the instance is in the symbol cache
    auto instanceUrl = instanceSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(instanceUrl))
        throwAssemblerInvariant("missing instance symbol {}", instanceUrl.toString());

    //
    auto location = instanceUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(instanceSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(instanceSymbol->instanceType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::InstanceSymbol *>::const_iterator
lyric_assembler::AssemblyState::instancesBegin() const
{
    return m_instances.cbegin();
}

std::vector<lyric_assembler::InstanceSymbol *>::const_iterator
lyric_assembler::AssemblyState::instancesEnd() const
{
    return m_instances.cend();
}

int
lyric_assembler::AssemblyState::numInstances() const
{
    return m_instances.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendEnum(EnumSymbol *enumSymbol)
{
    TU_ASSERT (enumSymbol != nullptr);
    auto symbolUrl = enumSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append enum; symbol {} already exists", symbolUrl.toString());
    m_enums.push_back(enumSymbol);
    m_symbolcache->insertSymbol(symbolUrl, enumSymbol);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::AssemblyState::touchEnum(EnumSymbol *enumSymbol)
{
    TU_ASSERT (enumSymbol != nullptr);

    // enum has been touched already, nothing to do
    if (enumSymbol->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the enum is in the symbol cache
    auto enumUrl = enumSymbol->getSymbolUrl();
    if (!m_symbolcache->hasSymbol(enumUrl))
        throwAssemblerInvariant("missing enum symbol {}", enumUrl.toString());

    //
    auto location = enumUrl.getAssemblyLocation();
    if (!m_importcache->hasImport(location))
        throwAssemblerInvariant("missing import {}", location.toString());
    auto status = m_importcache->touchImport(location);
    if (status.notOk())
        return status;

    status = m_importcache->linkFarSymbol(enumSymbol);
    if (status.notOk())
        return status;

    status = m_typecache->touchType(enumSymbol->enumType());
    if (status.notOk())
        return status;

    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::EnumSymbol *>::const_iterator
lyric_assembler::AssemblyState::enumsBegin() const
{
    return m_enums.cbegin();
}

std::vector<lyric_assembler::EnumSymbol *>::const_iterator
lyric_assembler::AssemblyState::enumsEnd() const
{
    return m_enums.cend();
}

int
lyric_assembler::AssemblyState::numEnums() const
{
    return m_enums.size();
}

tempo_utils::Status
lyric_assembler::AssemblyState::appendUndeclared(UndeclaredSymbol *undeclaredSymbol)
{
    TU_ASSERT (undeclaredSymbol != nullptr);
    auto symbolUrl = undeclaredSymbol->getSymbolUrl();
    if (m_symbolcache->hasSymbol(symbolUrl))
        throwAssemblerInvariant("failed to append undeclared; symbol {} already exists", symbolUrl.toString());
    m_undecls.push_back(undeclaredSymbol);
    m_symbolcache->insertSymbol(symbolUrl, undeclaredSymbol);
    return AssemblerStatus::ok();
}

std::vector<lyric_assembler::UndeclaredSymbol *>::const_iterator
lyric_assembler::AssemblyState::undeclaredBegin() const
{
    return m_undecls.cbegin();
}

std::vector<lyric_assembler::UndeclaredSymbol *>::const_iterator
lyric_assembler::AssemblyState::undeclaredEnd() const
{
    return m_undecls.cend();
}

int
lyric_assembler::AssemblyState::numUndeclared() const
{
    return m_undecls.size();
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_assembler::AssemblyState::toAssembly() const
{
    return internal::write_object(this);
}
