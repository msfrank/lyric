
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/base_invokable_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::BaseInvokableHandler::BaseInvokableHandler(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseGrouping(invokeBlock, driver),
      m_bindingBlock(bindingBlock),
      m_invokeBlock(invokeBlock),
      m_fragment(fragment)
{
    TU_ASSERT (m_bindingBlock != nullptr);
    TU_ASSERT (m_invokeBlock != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

lyric_assembler::BlockHandle *
lyric_compiler::BaseInvokableHandler::getBindingBlock() const
{
    return m_bindingBlock;
}

lyric_assembler::BlockHandle *
lyric_compiler::BaseInvokableHandler::getInvokeBlock() const
{
    return m_invokeBlock;
}

lyric_assembler::CodeFragment *
lyric_compiler::BaseInvokableHandler::getFragment() const
{
    return m_fragment;
}

tempo_utils::Status
lyric_compiler::BaseInvokableHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    for (int i = 0; i < node->numChildren(); i++) {
        auto argument = std::make_unique<Argument>();
        argument->fragment = m_fragment->makeFragment();
        auto invokable = std::make_unique<InvokableArgument>(
            argument.get(), i, &m_invocation, block, driver);
        m_invocation.arguments.push_back(std::move(argument));
        ctx.appendChoice(std::move(invokable));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::BaseInvokableHandler::placeArguments(
    const lyric_assembler::AbstractPlacement *placement,
    lyric_typing::CallsiteReifier &reifier,
    lyric_assembler::CodeFragment *fragment)
{
    absl::flat_hash_set<lyric_common::SymbolUrl> usedEvidence;
    ssize_t currpos = 0;
    ssize_t firstListOptArgument = -1;

    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *state = driver->getState();
    auto *symbolCache = state->symbolCache();

    auto numArguments = m_invocation.arguments.size();
    for (int i = numArguments - 1; 0 <= i; i--) {
        auto &argument = m_invocation.arguments.at(i);
        argument->resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
    }

    // push list arguments onto the stack, and record their types
    for (auto param = placement->listPlacementBegin(); param != placement->listPlacementEnd(); param++) {

        switch (param->placement) {

            // evaluate the list parameter
            case lyric_object::PlacementType::List: {
                if (m_invocation.listOffsets.size() <= currpos)
                    return state->logAndContinue(CompilerCondition::kUnexpectedArgument,
                        tempo_tracing::LogSeverity::kError,
                        "unexpected list argument {}", currpos);
                if (firstListOptArgument >= 0)
                    return state->logAndContinue(CompilerCondition::kUnexpectedArgument,
                        tempo_tracing::LogSeverity::kError,
                        "unexpected list argument {}; missing default initializer", currpos);
                auto offset = m_invocation.listOffsets.at(currpos);
                TU_ASSERT (offset < m_invocation.arguments.size());
                auto &argument = m_invocation.arguments.at(offset);
                TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
                TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(argument->resultType));
                currpos++;
                break;
            }

                // evaluate the list optional parameter
            case lyric_object::PlacementType::ListOpt: {
                if (m_invocation.listOffsets.size() <= currpos) {
                    if (!placement->hasInitializer(param->name))
                        return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                            tempo_tracing::LogSeverity::kError,
                            "invalid list parameter {}", param->name);
                    auto initializerUrl = placement->getInitializer(param->name);

                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(initializerUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
                        return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                            tempo_tracing::LogSeverity::kError,
                            "invalid initializer {}", initializerUrl.toString());
                    auto *initSymbol = cast_symbol_to_call(symbol);

                    auto callable = std::make_unique<lyric_assembler::FunctionCallable>(
                        initSymbol, /* isInlined= */ false);
                    lyric_assembler::CallableInvoker invoker;
                    TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));

                    lyric_typing::CallsiteReifier initReifier(typeSystem);
                    TU_RETURN_IF_NOT_OK (initReifier.initialize(invoker, reifier.getCallsiteArguments()));

                    lyric_common::TypeDef initializerType;
                    TU_ASSIGN_OR_RETURN (initializerType, invoker.invoke(getInvokeBlock(), initReifier, fragment));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(initializerType));
                } else {
                    auto offset = m_invocation.listOffsets.at(currpos);
                    TU_ASSERT (offset < m_invocation.arguments.size());
                    auto &argument = m_invocation.arguments.at(offset);
                    TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(argument->resultType));
                }
                if (firstListOptArgument < 0) {
                    firstListOptArgument = currpos;
                }
                currpos++;
                break;
            }

            default:
                return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                    tempo_tracing::LogSeverity::kError,
                    "invalid list parameter");
        }
    }

    // push argument values onto the stack, record their types
    for (auto param = placement->namedPlacementBegin(); param != placement->namedPlacementEnd(); param++) {

        switch (param->placement) {

            // evaluate the named parameter
            case lyric_object::PlacementType::Named: {
                auto entry = m_invocation.keywordOffsets.find(param->name);
                if (entry == m_invocation.keywordOffsets.cend())
                    return state->logAndContinue(CompilerCondition::kMissingArgument,
                        tempo_tracing::LogSeverity::kError,
                        "missing keyword argument '{}'", param->name);
                auto offset = entry->second;
                TU_ASSERT (offset < m_invocation.arguments.size());
                auto &argument = m_invocation.arguments.at(offset);
                TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
                TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(argument->resultType));
                break;
            }

            // evaluate the named optional parameter
            case lyric_object::PlacementType::NamedOpt: {
                auto entry = m_invocation.keywordOffsets.find(param->name);
                if (entry == m_invocation.keywordOffsets.cend()) {
                    if (!placement->hasInitializer(param->name))
                        return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                            tempo_tracing::LogSeverity::kError,
                            "invalid named parameter {}", param->name);
                    auto initializerUrl = placement->getInitializer(param->name);

                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(initializerUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
                        return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                            tempo_tracing::LogSeverity::kError,
                            "invalid initializer {}", initializerUrl.toString());
                    auto *initSymbol = cast_symbol_to_call(symbol);

                    auto callable = std::make_unique<lyric_assembler::FunctionCallable>(
                        initSymbol, /* isInlined= */ false);
                    lyric_assembler::CallableInvoker invoker;
                    TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));

                    lyric_typing::CallsiteReifier initReifier(typeSystem);
                    TU_RETURN_IF_NOT_OK (initReifier.initialize(invoker, reifier.getCallsiteArguments()));

                    lyric_common::TypeDef initializerType;
                    TU_ASSIGN_OR_RETURN (initializerType, invoker.invoke(getInvokeBlock(), initReifier, fragment));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(initializerType));
                } else {
                    auto offset = entry->second;
                    TU_ASSERT (offset < m_invocation.arguments.size());
                    auto &argument = m_invocation.arguments.at(offset);
                    TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(argument->resultType));
                }
                break;
            }

            // evaluate the ctx parameter
            case lyric_object::PlacementType::Ctx: {
                lyric_common::TypeDef contextType;
                TU_ASSIGN_OR_RETURN (contextType, reifier.reifyNextContext());
                auto entry = m_invocation.keywordOffsets.find(param->name);

                if (entry == m_invocation.keywordOffsets.cend()) {
                    lyric_common::SymbolUrl evidenceUrl;
                    TU_ASSIGN_OR_RETURN (evidenceUrl, m_bindingBlock->resolveImpl(contextType));
                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(evidenceUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
                        return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                            tempo_tracing::LogSeverity::kError,
                            "invalid ctx symbol {}", evidenceUrl.toString());
                    auto *instanceSymbol = cast_symbol_to_instance(symbol);
                    TU_RETURN_IF_NOT_OK (fragment->loadData(instanceSymbol));
                    usedEvidence.insert(evidenceUrl);
                }
                else {
                    auto offset = entry->second;
                    TU_ASSERT (offset < m_invocation.arguments.size());
                    auto &argument = m_invocation.arguments.at(offset);
                    TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
                    bool isImplementable;
                    TU_ASSIGN_OR_RETURN (isImplementable,
                        typeSystem->isImplementable(contextType, argument->resultType));
                    if (!isImplementable)
                        return state->logAndContinue(CompilerCondition::kMissingImpl,
                            tempo_tracing::LogSeverity::kError,
                            "ctx argument {} does not implement {}",
                            argument->resultType.toString(), contextType.toString());
                }
                break;
            }

            default:
                return state->logAndContinue(CompilerCondition::kCompilerInvariant,
                    tempo_tracing::LogSeverity::kError,
                    "invalid named parameter");
        }
    }

    // after explicit parameter placement, evaluate any remaining arguments as part of the rest parameter
    for (; currpos < m_invocation.listOffsets.size(); currpos++) {
        auto offset = m_invocation.listOffsets.at(currpos);
        TU_ASSERT (offset < m_invocation.arguments.size());
        auto &argument = m_invocation.arguments.at(offset);
        TU_RETURN_IF_NOT_OK (fragment->appendFragment(std::move(argument->fragment)));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(argument->resultType));
    }

    return {};
}

lyric_compiler::InvokableArgument::InvokableArgument(
    lyric_compiler::Argument *argument,
    tu_uint32 offset,
    lyric_compiler::Invocation *invocation,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_argument(argument),
      m_offset(offset),
      m_invocation(invocation)
{
    TU_ASSERT (m_argument != nullptr);
    TU_ASSERT (m_invocation != nullptr);
}

tempo_utils::Status
lyric_compiler::InvokableArgument::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid argument node");
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    auto astId = resource->getId();

    TU_LOG_INFO << "enter ArgumentHandler@" << this << ": " << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_argument->fragment.get();
    //TU_RETURN_IF_NOT_OK (driver->pushFragment(m_argument->fragment.get()));

    switch (astId) {

        //
        case lyric_schema::LyricAstId::Keyword: {
            std::string identifier;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
            m_invocation->keywordOffsets[identifier] = m_offset;
            auto keyword = std::make_unique<KeywordArgument>(fragment, block, driver);
            ctx.setGrouping(std::move(keyword));
            return {};
        }

        //
        default: {
            m_invocation->listOffsets.push_back(m_offset);
            auto expression = std::make_unique<FormChoice>(
                FormType::Expression, fragment, block, driver);
            ctx.setChoice(std::move(expression));
            return {};
        }
    }
}

lyric_compiler::KeywordArgument::KeywordArgument(
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::KeywordArgument::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto expression = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(expression));
    return {};
}