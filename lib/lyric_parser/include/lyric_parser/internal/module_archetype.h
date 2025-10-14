#ifndef LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H
#define LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"
#include "../parse_result.h"

namespace lyric_parser::internal {

    class ModuleArchetype : public ModuleParserBaseListener {

    public:
        ModuleArchetype(ArchetypeState *state, std::shared_ptr<tempo_tracing::TraceContext> context);
        virtual ~ModuleArchetype() = default;

        ArchetypeState *getState() const;

        void logErrorOrThrow(
            size_t lineNr,
            size_t columnNr,
            const std::string &message);

        bool hasError() const;

        void enterRoot(ModuleParser::RootContext *ctx) override;
        void enterBlock(ModuleParser::BlockContext *ctx) override;
        void exitForm(ModuleParser::FormContext *ctx) override;
        void exitRoot(ModuleParser::RootContext *ctx) override;

        void exitSymbolIdentifier(ModuleParser::SymbolIdentifierContext *ctx) override;

        // implemented by ModuleSymbolOps
        void enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx) override;
        void exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx) override;
        void exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx) override;

        void enterUsingStatement(ModuleParser::UsingStatementContext *ctx) override;
        void exitUsingRef(ModuleParser::UsingRefContext *ctx) override;
        void exitUsingType(ModuleParser::UsingTypeContext *ctx) override;
        void exitUsingStatement(ModuleParser::UsingStatementContext *ctx) override;

        void exitImportRef(ModuleParser::ImportRefContext *ctx) override;
        void exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx) override;
        void exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx) override;
        void enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx) override;
        void exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx) override;

        // implemented by ModuleConstantOps
        void exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx) override;
        void exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx) override;
        void exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx) override;
        void exitNilLiteral(ModuleParser::NilLiteralContext *ctx) override;
        void exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx) override;
        void exitHexInteger(ModuleParser::HexIntegerContext *ctx) override;
        void exitOctalInteger(ModuleParser::OctalIntegerContext *ctx) override;
        void exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx) override;
        void exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx) override;
        void exitHexFloat(ModuleParser::HexFloatContext *ctx) override;
        void exitInvalidNumber(ModuleParser::InvalidNumberContext *ctx) override;
        void exitCharLiteral(ModuleParser::CharLiteralContext *ctx) override;
        void exitStringLiteral(ModuleParser::StringLiteralContext *ctx) override;
        void exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx) override;

        // implemented by ModuleLogicalOps
        void exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx) override;
        void exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx) override;
        void exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx) override;

        // implemented by ModuleArithmeticOps
        void exitAddExpression(ModuleParser::AddExpressionContext *ctx) override;
        void exitSubExpression(ModuleParser::SubExpressionContext *ctx) override;
        void exitMulExpression(ModuleParser::MulExpressionContext *ctx) override;
        void exitDivExpression(ModuleParser::DivExpressionContext *ctx) override;
        void exitNegExpression(ModuleParser::NegExpressionContext *ctx) override;

        // implemented by ModuleCompareOps
        void exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx) override;
        void exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx) override;
        void exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx) override;
        void exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx) override;
        void exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx) override;

        // implemented by ModuleAssignOps
        void enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx) override;
        void exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx) override;
        void enterUntypedVal(ModuleParser::UntypedValContext *ctx) override;
        void exitUntypedVal(ModuleParser::UntypedValContext *ctx) override;
        void enterTypedVal(ModuleParser::TypedValContext *ctx) override;
        void exitTypedVal(ModuleParser::TypedValContext *ctx) override;
        void enterUntypedVar(ModuleParser::UntypedVarContext *ctx) override;
        void exitUntypedVar(ModuleParser::UntypedVarContext *ctx) override;
        void enterTypedVar(ModuleParser::TypedVarContext *ctx) override;
        void exitTypedVar(ModuleParser::TypedVarContext *ctx) override;
        void exitNameAssignment(ModuleParser::NameAssignmentContext *ctx) override;
        void exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx) override;
        void exitSetStatement(ModuleParser::SetStatementContext *ctx) override;

        // implemented by ModuleControlOps
        void exitIfStatement(ModuleParser::IfStatementContext *ctx) override;
        void exitTernaryExpression(ModuleParser::TernaryExpressionContext *ctx) override;
        void enterCondExpression(ModuleParser::CondExpressionContext *ctx) override;
        void exitCondWhen(ModuleParser::CondWhenContext *ctx) override;
        void exitCondElse(ModuleParser::CondElseContext *ctx) override;
        void enterDoStatement(ModuleParser::DoStatementContext *ctx) override;
        void exitDoWhen(ModuleParser::DoWhenContext *ctx) override;
        void exitDoElse(ModuleParser::DoElseContext *ctx) override;
        void enterWhileStatement(ModuleParser::WhileStatementContext *ctx) override;
        void exitWhileStatement(ModuleParser::WhileStatementContext *ctx) override;
        void enterForStatement(ModuleParser::ForStatementContext *ctx) override;
        void exitForStatement(ModuleParser::ForStatementContext * ctx) override;
        void exitReturnStatement(ModuleParser::ReturnStatementContext *ctx) override;

        // implemented by ModuleMatchOps
        void enterMatchExpression(ModuleParser::MatchExpressionContext *ctx) override;
        void exitMatchTarget(ModuleParser::MatchTargetContext *ctx) override;
        void exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx) override;
        void exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx) override;
        void exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx) override;
        void exitMatchElse(ModuleParser::MatchElseContext *ctx) override;

        // implemented by ModuleExceptionOps
        void enterTryStatement(ModuleParser::TryStatementContext *ctx) override;
        void exitTryBlock(ModuleParser::TryBlockContext *ctx) override;
        void enterTryCatch(ModuleParser::TryCatchContext *ctx) override;
        void exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx) override;
        void exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx) override;
        void exitCatchElse(ModuleParser::CatchElseContext *ctx) override;
        void exitTryCatch(ModuleParser::TryCatchContext *ctx) override;
        void enterTryFinally(ModuleParser::TryFinallyContext *ctx) override;
        void exitTryFinally(ModuleParser::TryFinallyContext *ctx) override;
        void exitExpectExpression(ModuleParser::ExpectExpressionContext *ctx) override;
        void exitRaiseExpression(ModuleParser::RaiseExpressionContext *ctx) override;

        // implemented by ModuleDerefOps
        void enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx) override;
        void enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx) override;
        void enterNewExpression(ModuleParser::NewExpressionContext *ctx) override;
        void enterThisExpression(ModuleParser::ThisExpressionContext *ctx) override;
        void enterNameExpression(ModuleParser::NameExpressionContext *ctx) override;
        void enterCallExpression(ModuleParser::CallExpressionContext *ctx) override;
        void exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx) override;
        void exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx) override;
        void exitThisSpec(ModuleParser::ThisSpecContext *ctx) override;
        void exitNameSpec(ModuleParser::NameSpecContext *ctx) override;
        void exitCallSpec(ModuleParser::CallSpecContext *ctx) override;
        void exitDerefMethod(ModuleParser::DerefMethodContext *ctx) override;
        void exitDerefMember(ModuleParser::DerefMemberContext *ctx) override;
        void exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx) override;
        void exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx) override;
        void exitThisExpression(ModuleParser::ThisExpressionContext *ctx) override;
        void exitNameExpression(ModuleParser::NameExpressionContext *ctx) override;
        void exitCallExpression(ModuleParser::CallExpressionContext *ctx) override;
        void exitSymbolExpression(ModuleParser::SymbolExpressionContext *ctx) override;
        void exitTypeofExpression(ModuleParser::TypeofExpressionContext *ctx) override;

        // implemented by ModuleConstructOps
        void exitDerefNew(ModuleParser::DerefNewContext *ctx) override;
        void exitDefaultThisBase(ModuleParser::DefaultThisBaseContext *ctx) override;
        void exitNamedThisBase(ModuleParser::NamedThisBaseContext *ctx) override;
        void exitDefaultSuperBase(ModuleParser::DefaultSuperBaseContext *ctx) override;
        void exitNamedSuperBase(ModuleParser::NamedSuperBaseContext *ctx) override;
        void exitPairExpression(ModuleParser::PairExpressionContext *ctx) override;
        void exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx) override;
        void exitLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx) override;
        void exitInitializerDefaultNew(ModuleParser::InitializerDefaultNewContext *ctx) override;
        void exitInitializerNamedNew(ModuleParser::InitializerNamedNewContext *ctx) override;

        // implemented by ModuleDefineOps
        void exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx) override;
        void enterDefStatement(ModuleParser::DefStatementContext *ctx) override;
        void exitDefStatement(ModuleParser::DefStatementContext *ctx) override;
        void enterImplDef(ModuleParser::ImplDefContext *ctx) override;
        void exitImplDef(ModuleParser::ImplDefContext *ctx) override;
        void exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx) override;

        // implemented by ModuleDefclassOps
        void enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx) override;
        void enterClassInit(ModuleParser::ClassInitContext *ctx) override;
        void exitClassInit(ModuleParser::ClassInitContext *ctx) override;
        void enterClassVal(ModuleParser::ClassValContext *ctx) override;
        void exitClassVal(ModuleParser::ClassValContext *ctx) override;
        void enterClassVar(ModuleParser::ClassVarContext *ctx) override;
        void exitClassVar(ModuleParser::ClassVarContext *ctx) override;
        void enterClassDef(ModuleParser::ClassDefContext *ctx) override;
        void exitClassDef(ModuleParser::ClassDefContext *ctx) override;
        void enterClassDecl(ModuleParser::ClassDeclContext *ctx) override;
        void exitClassDecl(ModuleParser::ClassDeclContext *ctx) override;
        void enterClassImpl(ModuleParser::ClassImplContext *ctx) override;
        void exitClassImpl(ModuleParser::ClassImplContext *ctx) override;
        void exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx) override;

        // implemented by ModuleDefconceptOps
        void enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx) override;
        void enterConceptDecl(ModuleParser::ConceptDeclContext *ctx) override;
        void exitConceptDecl(ModuleParser::ConceptDeclContext *ctx) override;
        void enterConceptImpl(ModuleParser::ConceptImplContext *ctx) override;
        void exitConceptImpl(ModuleParser::ConceptImplContext *ctx) override;
        void exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx) override;

        // implemented by ModuleDefenumOps
        void enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx) override;
        void enterEnumInit(ModuleParser::EnumInitContext *ctx) override;
        void exitEnumInit(ModuleParser::EnumInitContext *ctx) override;
        void enterEnumVal(ModuleParser::EnumValContext *ctx) override;
        void exitEnumVal(ModuleParser::EnumValContext *ctx) override;
        void enterEnumDef(ModuleParser::EnumDefContext *ctx) override;
        void exitEnumDef(ModuleParser::EnumDefContext *ctx) override;
        void enterEnumCase(ModuleParser::EnumCaseContext *ctx) override;
        void exitEnumCase(ModuleParser::EnumCaseContext *ctx) override;
        void enterEnumImpl(ModuleParser::EnumImplContext *ctx) override;
        void exitEnumImpl(ModuleParser::EnumImplContext *ctx) override;
        void exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx) override;

        // implemented by ModuleDefinstanceOps
        void enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx) override;
        void enterInstanceInit(ModuleParser::InstanceInitContext *ctx) override;
        void exitInstanceInit(ModuleParser::InstanceInitContext *ctx) override;
        void enterInstanceVal(ModuleParser::InstanceValContext *ctx) override;
        void exitInstanceVal(ModuleParser::InstanceValContext *ctx) override;
        void enterInstanceVar(ModuleParser::InstanceVarContext *ctx) override;
        void exitInstanceVar(ModuleParser::InstanceVarContext *ctx) override;
        void enterInstanceDef(ModuleParser::InstanceDefContext *ctx) override;
        void exitInstanceDef(ModuleParser::InstanceDefContext *ctx) override;
        void enterInstanceImpl(ModuleParser::InstanceImplContext *ctx) override;
        void exitInstanceImpl(ModuleParser::InstanceImplContext *ctx) override;
        void exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx) override;

        // implemented by ModuleDefstructOps
        void enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx) override;
        void enterStructInit(ModuleParser::StructInitContext *ctx) override;
        void exitStructInit(ModuleParser::StructInitContext *ctx) override;
        void enterStructVal(ModuleParser::StructValContext *ctx) override;
        void exitStructVal(ModuleParser::StructValContext *ctx) override;
        void enterStructDef(ModuleParser::StructDefContext *ctx) override;
        void exitStructDef(ModuleParser::StructDefContext *ctx) override;
        void enterStructImpl(ModuleParser::StructImplContext *ctx) override;
        void exitStructImpl(ModuleParser::StructImplContext *ctx) override;
        void exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx) override;

        // implemented by ModuleParameterOps
        void enterParamSpec(ModuleParser::ParamSpecContext *ctx) override;
        void exitPositionalParam(ModuleParser::PositionalParamContext *ctx) override;
        void exitNamedParam(ModuleParser::NamedParamContext *ctx) override;
        void exitRenamedParam(ModuleParser::RenamedParamContext *ctx) override;
        void exitNamedCtx(ModuleParser::NamedCtxContext *ctx) override;
        void exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx) override;
        void exitRest(ModuleParser::RestContext *ctx) override;
        void exitParamSpec(ModuleParser::ParamSpecContext *ctx) override;

        // implemented by ModuleMacroOps
        void exitMacroArgs(ModuleParser::MacroArgsContext *ctx) override;
        void enterMacroCall(ModuleParser::MacroCallContext *ctx) override;
        void exitMacroCall(ModuleParser::MacroCallContext *ctx) override;
        void enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx) override;
        void exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx) override;
        void enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx) override;
        void exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx) override;
        void enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx) override;
        void exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx) override;
        void enterBlockMacro(ModuleParser::BlockMacroContext *ctx) override;
        void exitBlockMacro(ModuleParser::BlockMacroContext *ctx) override;

        tempo_utils::Result<LyricArchetype> toArchetype() const;

    private:
        ArchetypeState *m_state;
        std::shared_ptr<tempo_tracing::TraceContext> m_context;
        tempo_utils::Status m_status;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H
