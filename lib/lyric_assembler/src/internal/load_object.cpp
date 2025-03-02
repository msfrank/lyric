
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/load_object.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>

tempo_utils::Result<lyric_common::ModuleLocation>
lyric_assembler::internal::find_system_bootstrap(
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
    ObjectState *state)
{
    TU_ASSERT (moduleImport != nullptr);
    TU_ASSERT (state != nullptr);

    auto object = moduleImport->getObject();
    auto root = object.getObject();

    for (int i = 0; i < root.numImports(); i++) {
        auto importWalker = root.getImport(i);
        if (importWalker.isSystemBootstrap()) {
            return importWalker.getImportLocation();
        }
    }
    return state->logAndContinue(AssemblerCondition::kAssemblerInvariant,
        tempo_tracing::LogSeverity::kError,
        "object {} is missing system bootstrap",
        moduleImport->getLocation().toString());
}

tempo_utils::Status
lyric_assembler::internal::load_object_symbols(
    std::shared_ptr<lyric_importer::ModuleImport> moduleImport,
    ObjectState *state)
{
    TU_ASSERT (moduleImport != nullptr);
    TU_ASSERT (state != nullptr);

    auto object = moduleImport->getObject();
    auto root = object.getObject();

    for (int i = 0; i < root.numActions(); i++) {
        auto walker = root.getAction(i);
        lyric_common::SymbolUrl actionUrl(walker.getSymbolPath());
        auto *actionImport = moduleImport->getAction(i);
        auto actionSymbol = std::make_unique<ActionSymbol>(actionUrl, actionImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendAction(actionSymbol.release()));
    }

    for (int i = 0; i < root.numBindings(); i++) {
        auto walker = root.getBinding(i);
        lyric_common::SymbolUrl bindingUrl(walker.getSymbolPath());
        auto *bindingImport = moduleImport->getBinding(i);
        auto bindingSymbol = std::make_unique<BindingSymbol>(bindingUrl, bindingImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendBinding(bindingSymbol.release()));
    }

    for (int i = 0; i < root.numCalls(); i++) {
        auto walker = root.getCall(i);
        lyric_common::SymbolUrl callUrl(walker.getSymbolPath());
        auto *callImport = moduleImport->getCall(i);
        auto callSymbol = std::make_unique<CallSymbol>(callUrl, callImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendCall(callSymbol.release()));
    }

    for (int i = 0; i < root.numClasses(); i++) {
        auto walker = root.getClass(i);
        lyric_common::SymbolUrl classUrl(walker.getSymbolPath());
        auto *classImport = moduleImport->getClass(i);
        auto classSymbol = std::make_unique<ClassSymbol>(classUrl, classImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendClass(classSymbol.release()));
    }

    for (int i = 0; i < root.numConcepts(); i++) {
        auto walker = root.getConcept(i);
        lyric_common::SymbolUrl conceptUrl(walker.getSymbolPath());
        auto *conceptImport = moduleImport->getConcept(i);
        auto conceptSymbol = std::make_unique<ConceptSymbol>(conceptUrl, conceptImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendConcept(conceptSymbol.release()));
    }

    for (int i = 0; i < root.numEnums(); i++) {
        auto walker = root.getEnum(i);
        lyric_common::SymbolUrl enumUrl(walker.getSymbolPath());
        auto *enumImport = moduleImport->getEnum(i);
        auto enumSymbol = std::make_unique<EnumSymbol>(enumUrl, enumImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendEnum(enumSymbol.release()));
    }

    for (int i = 0; i < root.numExistentials(); i++) {
        auto walker = root.getExistential(i);
        lyric_common::SymbolUrl existentialUrl(walker.getSymbolPath());
        auto *existentialImport = moduleImport->getExistential(i);
        auto existentialSymbol = std::make_unique<ExistentialSymbol>(existentialUrl, existentialImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendExistential(existentialSymbol.release()));
    }

    for (int i = 0; i < root.numFields(); i++) {
        auto walker = root.getField(i);
        lyric_common::SymbolUrl fieldUrl(walker.getSymbolPath());
        auto *fieldImport = moduleImport->getField(i);
        auto fieldSymbol = std::make_unique<FieldSymbol>(fieldUrl, fieldImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendField(fieldSymbol.release()));
    }

    for (int i = 0; i < root.numInstances(); i++) {
        auto walker = root.getInstance(i);
        lyric_common::SymbolUrl instanceUrl(walker.getSymbolPath());
        auto *instanceImport = moduleImport->getInstance(i);
        auto instanceSymbol = std::make_unique<InstanceSymbol>(instanceUrl, instanceImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendInstance(instanceSymbol.release()));
    }

    for (int i = 0; i < root.numNamespaces(); i++) {
        auto walker = root.getNamespace(i);
        lyric_common::SymbolUrl namespaceUrl(walker.getSymbolPath());
        auto *namespaceImport = moduleImport->getNamespace(i);
        auto namespaceSymbol = std::make_unique<NamespaceSymbol>(namespaceUrl, namespaceImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendNamespace(namespaceSymbol.release()));
    }

    for (int i = 0; i < root.numStatics(); i++) {
        auto walker = root.getStatic(i);
        lyric_common::SymbolUrl staticUrl(walker.getSymbolPath());
        auto *staticImport = moduleImport->getStatic(i);
        auto staticSymbol = std::make_unique<StaticSymbol>(staticUrl, staticImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendStatic(staticSymbol.release()));
    }

    for (int i = 0; i < root.numStructs(); i++) {
        auto walker = root.getStruct(i);
        lyric_common::SymbolUrl structUrl(walker.getSymbolPath());
        auto *structImport = moduleImport->getStruct(i);
        auto structSymbol = std::make_unique<StructSymbol>(structUrl, structImport, /* isCopied= */ true, state);
        TU_RETURN_IF_NOT_OK (state->appendStruct(structSymbol.release()));
    }

    return {};
}