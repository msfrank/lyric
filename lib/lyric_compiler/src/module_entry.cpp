
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/module_entry.h>
#include <lyric_parser/lyric_parser.h>
#include "lyric_compiler/internal/compile_block.h"
#include "lyric_compiler/internal/compile_defclass.h"
#include "lyric_compiler/internal/compile_defenum.h"
#include "lyric_compiler/internal/compile_definstance.h"
#include "lyric_compiler/internal/compile_defstruct.h"
#include "lyric_compiler/internal/compile_defconcept.h"
#include "lyric_compiler/internal/compile_def.h"

lyric_compiler::ModuleEntry::ModuleEntry(lyric_assembler::AssemblyState *state)
    : m_state(state),
      m_root(nullptr),
      m_entry(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_compiler::ModuleEntry::~ModuleEntry()
{
    delete m_typeSystem;
}

lyric_assembler::AssemblyState *
lyric_compiler::ModuleEntry::getState() const
{
    return m_state;
}

tempo_utils::Status
lyric_compiler::ModuleEntry::initialize()
{
    if (m_typeSystem != nullptr)
        m_state->throwAssemblerInvariant("module entry is already initialized");
    m_typeSystem = new lyric_typing::TypeSystem(m_state);

    auto location = m_state->getLocation();
    if (!location.isValid())
        m_state->throwAssemblerInvariant("missing entry module location");

    // lookup the Function0 class and get its type handle
    auto functionClassUrl = m_state->fundamentalCache()->getFunctionUrl(0);
    if (!functionClassUrl.isValid())
        m_state->throwAssemblerInvariant("missing Function0 symbol");
    m_state->symbolCache()->touchSymbol(functionClassUrl);

    lyric_assembler::TypeCache *typeCache = m_state->typeCache();
    tempo_utils::Status status;

    // ensure that NoReturn is in the type cache
    auto returnType = lyric_common::TypeDef::noReturn();
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(returnType));
    typeCache->touchType(returnType);

    lyric_assembler::TypeHandle *entryTypeHandle;
    TU_ASSIGN_OR_RETURN (entryTypeHandle, typeCache->declareFunctionType(returnType, {}, {}));
    entryTypeHandle->touch();

    // create the $entry call
    lyric_common::SymbolUrl entryUrl(location, lyric_common::SymbolPath({"$entry"}));
    auto entryAddress = lyric_assembler::CallAddress::near(m_state->numCalls());
    m_entry = new lyric_assembler::CallSymbol(entryUrl, returnType, entryAddress, entryTypeHandle, m_state);
    status = m_state->appendCall(m_entry);
    if (status.notOk()) {
        delete m_entry;
        return status;
    }
    m_entry->touch();

    // resolve the Namespace type
    auto namespaceType = m_state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Namespace);
    lyric_assembler::TypeHandle *namespaceTypeHandle;
    TU_ASSIGN_OR_RETURN (namespaceTypeHandle, typeCache->getOrMakeType(namespaceType));

    // create the root namespace
    lyric_common::SymbolUrl rootUrl(location, lyric_common::SymbolPath({"$root"}));
    auto nsAddress = lyric_assembler::NamespaceAddress::near(m_state->numNamespaces());
    m_root = new lyric_assembler::NamespaceSymbol(rootUrl, nsAddress, namespaceTypeHandle,
        m_entry->callProc(), m_state);
    status = m_state->appendNamespace(m_root);
    if (status.notOk()) {
        delete m_root;
        return status;
    }

    return CompilerStatus::ok();
}

lyric_common::AssemblyLocation
lyric_compiler::ModuleEntry::getLocation() const
{
    return m_state->getLocation();
}

lyric_assembler::NamespaceSymbol *
lyric_compiler::ModuleEntry::getRoot() const
{
    return m_root;
}

lyric_assembler::CallSymbol *
lyric_compiler::ModuleEntry::getEntry() const
{
    return m_entry;
}

lyric_typing::TypeSystem *
lyric_compiler::ModuleEntry::getTypeSystem() const
{
    return m_typeSystem;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::ModuleEntry::compileBlock(std::string_view utf8, lyric_assembler::BlockHandle *block)
{
    // parse the block code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseBlock(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the block code
    return internal::compile_block(block, archetype.getRoot(), *this);
}

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_compiler::ModuleEntry::compileClass(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the defclass code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseClass(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the defclass code
    lyric_assembler::ClassSymbol *classSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_defclass(block, archetype.getRoot(), *this, &classSymbol));
    return classSymbol;
}

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_compiler::ModuleEntry::compileConcept(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the defconcept code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseConcept(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the defconcept code
    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_defconcept(block, archetype.getRoot(), *this, &conceptSymbol));
    return conceptSymbol;
}

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_compiler::ModuleEntry::compileEnum(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the defenum code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseEnum(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the defenum code
    lyric_assembler::EnumSymbol *enumSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_defenum(block, archetype.getRoot(), *this, &enumSymbol));
    return enumSymbol;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_compiler::ModuleEntry::compileFunction(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the def code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseFunction(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the def code
    lyric_assembler::CallSymbol *callSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_def(block, archetype.getRoot(), *this, &callSymbol));
    return callSymbol;
}

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_compiler::ModuleEntry::compileInstance(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the definstance code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseInstance(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the definstance code
    lyric_assembler::InstanceSymbol *instanceSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_definstance(block, archetype.getRoot(), *this, &instanceSymbol));
    return instanceSymbol;
}

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_compiler::ModuleEntry::compileStruct(
    std::string_view utf8,
    lyric_assembler::BlockHandle *block)
{
    // parse the defstruct code
    lyric_parser::ParserOptions parserOptions;
    lyric_parser::LyricParser lyricParser(parserOptions);
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, lyricParser.parseStruct(utf8, getLocation().toUrl(), m_state->scopeManager()));

    // compile the defstruct code
    lyric_assembler::StructSymbol *structSymbol;
    TU_RETURN_IF_NOT_OK (internal::compile_defstruct(block, archetype.getRoot(), *this, &structSymbol));
    return structSymbol;
}

//void
//lyric_compiler::ModuleEntry::putExitType(const lyric_common::TypeDef &exitType)
//{
//    m_entry->putExitType(exitType);
//}
//
//absl::flat_hash_set<lyric_common::TypeDef>
//lyric_compiler::ModuleEntry::listExitTypes() const
//{
//    return m_entry->listExitTypes();
//}