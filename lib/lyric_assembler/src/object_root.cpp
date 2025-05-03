
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::ObjectRoot::ObjectRoot(ObjectState *state)
    : m_state(state),
      m_globalNamespace(nullptr),
      m_entryCall(nullptr)
{
    TU_ASSERT (m_state != nullptr);
    m_preludeBlock = std::make_unique<BlockHandle>(m_state);
    m_rootBlock = std::make_unique<BlockHandle>(m_preludeBlock.get(), m_state);
}

tempo_utils::Status
lyric_assembler::ObjectRoot::initialize(std::shared_ptr<lyric_importer::ModuleImport> preludeImport)
{
    TU_ASSERT (preludeImport != nullptr);

    auto *fundamentalCache = m_state->fundamentalCache();
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto location = m_state->getLocation();

    // resolve the Namespace type
    auto namespaceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Namespace);
    lyric_assembler::TypeHandle *namespaceTypeHandle;
    TU_ASSIGN_OR_RETURN (namespaceTypeHandle, typeCache->getOrMakeType(namespaceType));

    // create the $global namespace
    lyric_common::SymbolUrl globalUrl(location, lyric_common::SymbolPath({"$global"}));
    auto globalNamespace = std::make_unique<lyric_assembler::NamespaceSymbol>(
        globalUrl, namespaceTypeHandle, m_rootBlock.get(), m_state);
    TU_RETURN_IF_NOT_OK (m_state->appendNamespace(globalNamespace.get()));
    m_globalNamespace = globalNamespace.release();

    // create the $entry call
    lyric_common::SymbolUrl entryUrl(location, lyric_common::SymbolPath({"$entry"}));
    auto entryCall = std::make_unique<lyric_assembler::CallSymbol>(
        entryUrl, m_rootBlock.get(), m_state);
    TU_RETURN_IF_NOT_OK (m_state->appendCall(entryCall.get()));
    m_entryCall = entryCall.release();

    auto preludeObject = preludeImport->getObject().getObject();
    auto preludeLocation = preludeImport->getObjectLocation();

    // import all prelude symbols into the root block
    for (int i = 0; i < preludeObject.numSymbols(); i++) {
        auto symbolWalker = preludeObject.getSymbol(i);
        TU_ASSERT (symbolWalker.isValid());

        auto symbolPath = symbolWalker.getSymbolPath();
        if (symbolPath.isEnclosed())
            continue;

        auto symbolName = symbolPath.getName();
        lyric_common::SymbolUrl symbolUrl(preludeLocation, symbolWalker.getSymbolPath());
        TU_ASSERT (symbolUrl.isValid());

        lyric_assembler::SymbolBinding binding;
        binding.bindingType = lyric_assembler::BindingType::Descriptor;
        binding.symbolUrl = symbolUrl;

        switch (symbolWalker.getLinkageSection()) {
            case lyric_object::LinkageSection::Call: {
                auto *callImport = preludeImport->getCall(symbolWalker.getLinkageIndex());
                if (!callImport->getReceiverUrl().isValid()) {
                    TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                }
                break;
            }
            case lyric_object::LinkageSection::Class: {
                auto *classImport = preludeImport->getClass(symbolWalker.getLinkageIndex());
                binding.typeDef = classImport->getClassType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }
            case lyric_object::LinkageSection::Concept: {
                auto *conceptImport = preludeImport->getConcept(symbolWalker.getLinkageIndex());
                binding.typeDef = conceptImport->getConceptType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }
            case lyric_object::LinkageSection::Enum: {
                auto *enumImport = preludeImport->getEnum(symbolWalker.getLinkageIndex());
                binding.typeDef = enumImport->getEnumType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }
            case lyric_object::LinkageSection::Existential: {
                auto *existentialImport = preludeImport->getExistential(symbolWalker.getLinkageIndex());
                binding.typeDef = existentialImport->getExistentialType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }
            case lyric_object::LinkageSection::Instance: {
                auto *instanceImport = preludeImport->getInstance(symbolWalker.getLinkageIndex());
                binding.typeDef = instanceImport->getInstanceType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                InstanceSymbol *instanceSymbol;
                TU_ASSIGN_OR_RETURN (instanceSymbol, importCache->importInstance(symbolUrl));
                TU_RETURN_IF_NOT_OK (m_preludeBlock->useImpls(instanceSymbol));
                break;
            }
            case lyric_object::LinkageSection::Static: {
                auto *staticImport = preludeImport->getStatic(symbolWalker.getLinkageIndex());
                binding.typeDef = staticImport->getStaticType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }
            case lyric_object::LinkageSection::Struct: {
                auto *structImport = preludeImport->getStruct(symbolWalker.getLinkageIndex());
                binding.typeDef = structImport->getStructType()->getTypeDef();
                TU_RETURN_IF_STATUS (m_preludeBlock->declareAlias(symbolName, binding));
                break;
            }

            default:
                break;
        }
    }

    return {};
}

lyric_assembler::BlockHandle *
lyric_assembler::ObjectRoot::rootBlock()
{
    return m_rootBlock.get();
}

lyric_assembler::NamespaceSymbol *
lyric_assembler::ObjectRoot::globalNamespace()
{
    return m_globalNamespace;
}

lyric_assembler::CallSymbol *
lyric_assembler::ObjectRoot::entryCall()
{
    return m_entryCall;
}