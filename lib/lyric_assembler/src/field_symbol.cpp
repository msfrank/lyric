
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/field_import.h>

lyric_assembler::FieldSymbol::FieldSymbol(
    const lyric_common::SymbolUrl &fieldUrl,
    lyric_object::AccessType access,
    bool isVariable,
    FieldAddress address,
    TypeHandle *fieldType,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new FieldSymbolPriv()),
      m_fieldUrl(fieldUrl),
      m_state(state)
{
    TU_ASSERT (m_fieldUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->isVariable = isVariable;
    priv->isDeclOnly = isDeclOnly;
    priv->fieldType = fieldType;
    priv->parentBlock = parentBlock;

    TU_ASSERT (priv->fieldType != nullptr);
    TU_ASSERT (priv->parentBlock != nullptr);
}

//lyric_assembler::FieldSymbol::FieldSymbol(
//    const lyric_common::SymbolUrl &fieldUrl,
//    lyric_object::AccessType access,
//    bool isVariable,
//    const lyric_common::SymbolUrl &init,
//    FieldAddress address,
//    TypeHandle *fieldType,
//    bool isDeclOnly,
//    BlockHandle *parentBlock,
//    AssemblyState *state)
//    : FieldSymbol(fieldUrl, access, isVariable, address, fieldType, isDeclOnly, parentBlock, state)
//{
//    auto *priv = getPriv();
//    priv->init = init;
//    TU_ASSERT (priv->init.isValid());
//}

lyric_assembler::FieldSymbol::FieldSymbol(
    const lyric_common::SymbolUrl &fieldUrl,
    lyric_importer::FieldImport *fieldImport,
    AssemblyState *state)
    : m_fieldUrl(fieldUrl),
      m_fieldImport(fieldImport),
      m_state(state)
{
    TU_ASSERT (m_fieldUrl.isValid());
    TU_ASSERT (m_fieldImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::FieldSymbolPriv *
lyric_assembler::FieldSymbol::load()
{
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<FieldSymbolPriv>();

    priv->parentBlock = nullptr;
    priv->access = m_fieldImport->getAccess();
    priv->isVariable = m_fieldImport->isVariable();
    priv->isDeclOnly = m_fieldImport->isDeclOnly();

    auto *fieldType = m_fieldImport->getFieldType();
    TU_ASSIGN_OR_RAISE (priv->fieldType, typeCache->importType(fieldType));

    priv->init = m_fieldImport->getInitializer();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::FieldSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Field;
}

lyric_assembler::SymbolType
lyric_assembler::FieldSymbol::getSymbolType() const
{
    return SymbolType::FIELD;
}

lyric_common::SymbolUrl
lyric_assembler::FieldSymbol::getSymbolUrl() const
{
    return m_fieldUrl;
}

lyric_common::TypeDef
lyric_assembler::FieldSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->fieldType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::FieldSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->fieldType->getTypeSignature();
}

void
lyric_assembler::FieldSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchField(this);
}

std::string
lyric_assembler::FieldSymbol::getName() const
{
    return m_fieldUrl.getSymbolPath().getName();
}

lyric_object::AccessType
lyric_assembler::FieldSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

bool
lyric_assembler::FieldSymbol::isVariable() const
{
    auto *priv = getPriv();
    return priv->isVariable;
}

bool
lyric_assembler::FieldSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_common::SymbolUrl
lyric_assembler::FieldSymbol::getInitializer() const
{
    auto *priv = getPriv();
    return priv->init;
}

tempo_utils::Result<lyric_assembler::ProcHandle *>
lyric_assembler::FieldSymbol::defineInitializer()
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't define initializer on imported field {}", m_fieldUrl.toString());
    auto *priv = getPriv();

    if (priv->init.isValid())
        m_state->throwAssemblerInvariant("cannot redefine initializer for {}", m_fieldUrl.toString());

    auto identifier = absl::StrCat("$init$", m_fieldUrl.getSymbolName());

    // declare the initializer call
    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, priv->parentBlock->declareFunction(
        identifier, lyric_object::AccessType::Public, {}, priv->isDeclOnly));

    priv->init = callSymbol->getSymbolUrl();

    // define the initializer with no parameters and the field type as return type
    return callSymbol->defineCall({}, priv->fieldType->getTypeDef());
}

lyric_assembler::DataReference
lyric_assembler::FieldSymbol::getReference() const
{
    auto *priv = getPriv();
    DataReference ref;
    ref.symbolUrl = m_fieldUrl;
    ref.typeDef = priv->fieldType->getTypeDef();
    ref.referenceType = priv->isVariable? ReferenceType::Variable : ReferenceType::Value;
    return ref;
}
