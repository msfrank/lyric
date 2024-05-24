
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_symbolizer/internal/entry_point.h>

lyric_symbolizer::internal::EntryPoint::EntryPoint(lyric_assembler::AssemblyState *state)
    : m_state(state),
      m_location(),
      m_root(nullptr),
      m_entry(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_state != nullptr);
    m_typeSystem = new lyric_typing::TypeSystem(m_state);
}

lyric_symbolizer::internal::EntryPoint::~EntryPoint()
{
    delete m_typeSystem;
}

lyric_assembler::AssemblyState *
lyric_symbolizer::internal::EntryPoint::getState() const
{
    return m_state;
}

tempo_utils::Status
lyric_symbolizer::internal::EntryPoint::initialize(const lyric_common::AssemblyLocation &location)
{
    if (!location.isValid())
        m_state->throwAssemblerInvariant("missing entry module location");
    if (m_location.isValid())
        m_state->throwAssemblerInvariant("module entry is already initialized");

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

    return SymbolizerStatus::ok();
}

lyric_common::AssemblyLocation
lyric_symbolizer::internal::EntryPoint::getLocation() const
{
    return m_location;
}

lyric_assembler::NamespaceSymbol *
lyric_symbolizer::internal::EntryPoint::getRoot() const
{
    return m_root;
}

lyric_assembler::CallSymbol *
lyric_symbolizer::internal::EntryPoint::getEntry() const
{
    return m_entry;
}

lyric_typing::TypeSystem *
lyric_symbolizer::internal::EntryPoint::getTypeSystem() const
{
    return m_typeSystem;
}

void
lyric_symbolizer::internal::EntryPoint::putExitType(const lyric_common::TypeDef &exitType)
{
    m_entry->putExitType(exitType);
}

absl::flat_hash_set<lyric_common::TypeDef>
lyric_symbolizer::internal::EntryPoint::listExitTypes() const
{
    return m_entry->listExitTypes();
}