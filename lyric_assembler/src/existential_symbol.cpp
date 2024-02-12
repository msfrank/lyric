
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_utils/log_message.h>

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    ExistentialSymbol *superExistential,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new ExistentialSymbolPriv()),
      m_existentialUrl(existentialUrl),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->existentialType = existentialType;
    priv->existentialTemplate = nullptr;
    priv->superExistential = superExistential;
    priv->existentialBlock = std::make_unique<BlockHandle>(existentialUrl, parentBlock, false);

    TU_ASSERT (priv->existentialType != nullptr);
    TU_ASSERT (priv->superExistential != nullptr);
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    TemplateHandle *existentialTemplate,
    ExistentialSymbol *superExistential,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : ExistentialSymbol(
        existentialUrl,
        access,
        derive,
        address,
        existentialType,
        superExistential,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->existentialTemplate = existentialTemplate;
    TU_ASSERT(priv->existentialTemplate != nullptr);
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_importer::ExistentialImport *existentialImport,
    AssemblyState *state)
    : m_existentialUrl(existentialUrl),
      m_existentialImport(existentialImport),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_existentialImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ExistentialSymbolPriv *
lyric_assembler::ExistentialSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ExistentialSymbolPriv>();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_existentialImport->getDerive();

    auto *existentialType = m_existentialImport->getExistentialType();
    TU_ASSIGN_OR_RAISE (priv->existentialType, typeCache->importType(existentialType));

    auto *existentialTemplate = m_existentialImport->getExistentialTemplate();
    if (existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->existentialTemplate, typeCache->importTemplate(existentialTemplate));
    }

    auto superExistentialUrl = m_existentialImport->getSuperExistential();
    if (superExistentialUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superExistential, importCache->importExistential(superExistentialUrl));
    }

    for (auto iterator = m_existentialImport->sealedTypesBegin(); iterator != m_existentialImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ExistentialSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Existential;
}

lyric_assembler::SymbolType
lyric_assembler::ExistentialSymbol::getSymbolType() const
{
    return SymbolType::EXISTENTIAL;
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialSymbol::getSymbolUrl() const
{
    return m_existentialUrl;
}

lyric_common::TypeDef
lyric_assembler::ExistentialSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ExistentialSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeSignature();
}

void
lyric_assembler::ExistentialSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchExistential(this);
}

lyric_object::DeriveType
lyric_assembler::ExistentialSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

lyric_assembler::ExistentialSymbol *
lyric_assembler::ExistentialSymbol::superExistential() const
{
    auto *priv = getPriv();
    return priv->superExistential;
}

lyric_assembler::TypeHandle *
lyric_assembler::ExistentialSymbol::existentialType() const
{
    auto *priv = getPriv();
    return priv->existentialType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ExistentialSymbol::existentialTemplate() const
{
    auto *priv = getPriv();
    return priv->existentialTemplate;
}

bool
lyric_assembler::ExistentialSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ExistentialSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "existential {} is not sealed", m_existentialUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(sealedUrl))
        m_state->throwAssemblerInvariant("missing symbol {}", sealedUrl.toString());
    auto *sym = m_state->symbolCache()->getSymbol(sealedType.getConcreteUrl());
    TU_ASSERT (sym != nullptr);

    TypeHandle *derivedType = nullptr;
    switch (sym->getSymbolType()) {
        case SymbolType::CLASS:
            derivedType = cast_symbol_to_class(sym)->classType();
            break;
        case SymbolType::ENUM:
            derivedType = cast_symbol_to_enum(sym)->enumType();
            break;
        case SymbolType::EXISTENTIAL:
            derivedType = cast_symbol_to_existential(sym)->existentialType();
            break;
        case SymbolType::INSTANCE:
            derivedType = cast_symbol_to_instance(sym)->instanceType();
            break;
        case SymbolType::STRUCT:
            derivedType = cast_symbol_to_struct(sym)->structType();
            break;
        default:
            break;
    }

    if (derivedType == nullptr || derivedType->getSuperType() != priv->existentialType)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return AssemblerStatus::ok();
}
