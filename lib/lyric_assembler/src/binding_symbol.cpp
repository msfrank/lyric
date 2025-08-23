
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::BindingSymbol::BindingSymbol(
    const lyric_common::SymbolUrl &bindingUrl,
    bool isHidden,
    TypeHandle *bindingType,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new BindingSymbolPriv()),
      m_bindingUrl(bindingUrl),
      m_state(state)
{
    TU_ASSERT (m_bindingUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->bindingType = bindingType;
    TU_ASSERT (priv->bindingType != nullptr);
    priv->parentBlock = parentBlock;
    TU_ASSERT (priv->parentBlock != nullptr);
}

lyric_assembler::BindingSymbol::BindingSymbol(
    const lyric_common::SymbolUrl &bindingUrl,
    bool isHidden,
    TypeHandle *bindingType,
    TemplateHandle *bindingTemplate,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new BindingSymbolPriv()),
      m_bindingUrl(bindingUrl),
      m_state(state)
{
    TU_ASSERT (m_bindingUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->bindingType = bindingType;
    TU_ASSERT (priv->bindingType != nullptr);
    priv->bindingTemplate = bindingTemplate;
    TU_ASSERT (priv->bindingTemplate != nullptr);
    priv->parentBlock = parentBlock;
    TU_ASSERT (priv->parentBlock != nullptr);
}

lyric_assembler::BindingSymbol::BindingSymbol(
    const lyric_common::SymbolUrl &bindingUrl,
    lyric_importer::BindingImport *bindingImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_bindingUrl(bindingUrl),
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
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<BindingSymbolPriv>();

    priv->isHidden = m_bindingImport->isHidden();

    auto *bindingType = m_bindingImport->getBindingType();
    TU_ASSIGN_OR_RAISE (priv->bindingType, typeCache->importType(bindingType));

    auto *bindingTemplate = m_bindingImport->getBindingTemplate();
    if (bindingTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->bindingTemplate, typeCache->importTemplate(bindingTemplate));
    }

    auto *targetType = m_bindingImport->getTargetType();
    TU_ASSIGN_OR_RAISE (priv->targetType, typeCache->importType(targetType));

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

bool
lyric_assembler::BindingSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

lyric_assembler::TypeHandle *
lyric_assembler::BindingSymbol::bindingType() const
{
    auto *priv = getPriv();
    return priv->bindingType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::BindingSymbol::bindingTemplate() const
{
    auto *priv = getPriv();
    return priv->bindingTemplate;
}

lyric_assembler::TypeHandle *
lyric_assembler::BindingSymbol::targetType() const
{
    auto *priv = getPriv();
    return priv->targetType;
}

lyric_assembler::AbstractResolver *
lyric_assembler::BindingSymbol::bindingResolver() const
{
    auto *priv = getPriv();
    if (priv->bindingTemplate != nullptr)
        return priv->bindingTemplate;
    return priv->parentBlock;
}

tempo_utils::Status
lyric_assembler::BindingSymbol::defineTarget(const lyric_common::TypeDef &targetType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't define target on imported binding {}", m_bindingUrl.toString());
    auto *priv = getPriv();
    if (priv->targetType != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "target is already defined");
    auto *typeCache = m_state->typeCache();
    TU_ASSIGN_OR_RETURN (priv->targetType, typeCache->getOrMakeType(targetType));
    return {};
}

static tempo_utils::Result<lyric_common::TypeDef>
resolve_target_recursive(
    const lyric_common::SymbolUrl &bindingUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments,
    const lyric_common::TypeDef &targetType)
{
    switch (targetType.getType()) {

        case lyric_common::TypeDefType::Placeholder: {
            if (targetType.getPlaceholderTemplateUrl() == bindingUrl) {
                auto placeholderIndex = targetType.getPlaceholderIndex();
                if (placeholderIndex < 0 || typeArguments.size() <= placeholderIndex)
                    return lyric_assembler::AssemblerStatus::forCondition(
                        lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                        "target placeholder has invalid index");
                return typeArguments.at(placeholderIndex);
            }
            return targetType;
        }

        case lyric_common::TypeDefType::Concrete: {
            auto span = targetType.getConcreteArguments();
            std::vector<lyric_common::TypeDef> concreteArguments(span.begin(), span.end());
            for (int i = 0; i < concreteArguments.size(); i++) {
                const auto &argType = concreteArguments.at(i);
                TU_ASSIGN_OR_RETURN (concreteArguments[i], resolve_target_recursive(
                    bindingUrl, typeArguments, argType));
            }
            return lyric_common::TypeDef::forConcrete(targetType.getConcreteUrl(), concreteArguments);
        }

        case lyric_common::TypeDefType::Union: {
            auto span = targetType.getUnionMembers();
            std::vector<lyric_common::TypeDef> unionMembers(span.begin(), span.end());
            for (int i = 0; i < unionMembers.size(); i++) {
                const auto &argType = unionMembers.at(i);
                TU_ASSIGN_OR_RETURN (unionMembers[i], resolve_target_recursive(
                    bindingUrl, typeArguments, argType));
            }
            return lyric_common::TypeDef::forUnion(unionMembers);
        }

        case lyric_common::TypeDefType::Intersection: {
            auto span = targetType.getIntersectionMembers();
            std::vector<lyric_common::TypeDef> intersectionMembers(span.begin(), span.end());
            for (int i = 0; i < intersectionMembers.size(); i++) {
                const auto &argType = intersectionMembers.at(i);
                TU_ASSIGN_OR_RETURN (intersectionMembers[i], resolve_target_recursive(
                    bindingUrl, typeArguments, argType));
            }
            return lyric_common::TypeDef::forIntersection(intersectionMembers);
        }

        case lyric_common::TypeDefType::NoReturn:
            return targetType;

        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid type");
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::BindingSymbol::resolveTarget(const std::vector<lyric_common::TypeDef> &typeArguments)
{
    auto *priv = getPriv();
    auto targetType = priv->targetType->getTypeDef();

    if (priv->bindingTemplate == nullptr) {
        if (!typeArguments.empty())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "binding does not accept type arguments");
        return targetType;
    }

    return resolve_target_recursive(m_bindingUrl, typeArguments, targetType);
}