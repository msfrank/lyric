
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>

lyric_assembler::BindingSymbol::BindingSymbol(
    const lyric_common::SymbolUrl &bindingUrl,
    const lyric_common::SymbolUrl &targetUrl,
    lyric_object::AccessType access,
    ObjectState *state)
    : BaseSymbol(new BindingSymbolPriv()),
      m_bindingUrl(bindingUrl),
      m_state(state)
{
    TU_ASSERT (m_bindingUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->targetUrl = targetUrl;
    priv->access = access;
}

lyric_assembler::BindingSymbol::BindingSymbol(
    const lyric_common::SymbolUrl &bindingUrl,
    lyric_importer::BindingImport *bindingImport,
    ObjectState *state)
    : m_bindingUrl(bindingUrl),
      m_bindingImport(bindingImport),
      m_state(state)
{
    TU_ASSERT (m_bindingUrl.isValid());
    TU_ASSERT (m_bindingImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::BindingSymbolPriv *
lyric_assembler::BindingSymbol::load()
{
    auto priv = std::make_unique<BindingSymbolPriv>();
    priv->access = m_bindingImport->getAccess();
    priv->targetUrl = m_bindingImport->getTargetUrl();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::BindingSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Binding;
}

lyric_assembler::SymbolType
lyric_assembler::BindingSymbol::getSymbolType() const
{
    return SymbolType::BINDING;
}

lyric_common::SymbolUrl
lyric_assembler::BindingSymbol::getSymbolUrl() const
{
    return m_bindingUrl;
}

lyric_common::TypeDef
lyric_assembler::BindingSymbol::getTypeDef() const
{
    return {};
}

std::string
lyric_assembler::BindingSymbol::getName() const
{
    return m_bindingUrl.getSymbolPath().getName();
}

lyric_object::AccessType
lyric_assembler::BindingSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_common::SymbolUrl
lyric_assembler::BindingSymbol::getTargetUrl() const
{
    auto *priv = getPriv();
    return priv->targetUrl;
}
