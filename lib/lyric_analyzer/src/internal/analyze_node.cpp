
#include <lyric_analyzer/internal/analyze_def.h>
#include <lyric_analyzer/internal/analyze_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Status
lyric_analyzer::internal::analyze_val(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = entryPoint.getTypeSystem();

    if (!block->isRoot())
        return AnalyzerStatus::ok();

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::NodeWalker typeNode;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec valSpec;
    TU_ASSIGN_OR_RETURN (valSpec, typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef valType;
    TU_ASSIGN_OR_RETURN (valType, typeSystem->resolveAssignable(block, valSpec));

    auto declareVariableResult = block->declareStatic(identifier,
        valType, lyric_parser::BindingType::VALUE, true);
    if (declareVariableResult.isStatus())
        return declareVariableResult.getStatus();
    return AnalyzerStatus::ok();
}

tempo_utils::Status
lyric_analyzer::internal::analyze_var(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = entryPoint.getTypeSystem();

    if (!block->isRoot())
        return AnalyzerStatus::ok();

    std::string identifier;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_parser::NodeWalker typeNode;
    entryPoint.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeNode);
    lyric_typing::TypeSpec varSpec;
    TU_ASSIGN_OR_RETURN (varSpec, typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef varType;
    TU_ASSIGN_OR_RETURN (varType, typeSystem->resolveAssignable(block, varSpec));

    auto declareVariableResult = block->declareStatic(identifier,
        varType, lyric_parser::BindingType::VARIABLE, true);
    if (declareVariableResult.isStatus())
        return declareVariableResult.getStatus();
    return AnalyzerStatus::ok();
}

tempo_utils::Status
lyric_analyzer::internal::analyze_block(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT(walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    for (int i = 0; i < walker.numChildren(); i++) {
        auto status = analyze_node(block, walker.getChild(i), entryPoint);
        if (!status.isOk())
            return status;
    }
    return AnalyzerStatus::ok();
}

tempo_utils::Status
lyric_analyzer::internal::analyze_node(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT(block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    entryPoint.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, nodeId);

    switch (nodeId) {

        // ignored literal forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
            return AnalyzerStatus::ok();

        // ignored expression forms
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::IsA:
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::Deref:
        case lyric_schema::LyricAstId::New:

        case lyric_schema::LyricAstId::Cond:
        case lyric_schema::LyricAstId::Match:
        case lyric_schema::LyricAstId::Lambda:
            return AnalyzerStatus::ok();

        // ignored statement forms
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::While:
        case lyric_schema::LyricAstId::For:
        case lyric_schema::LyricAstId::Return:
            return AnalyzerStatus::ok();

        case lyric_schema::LyricAstId::Block:
            return analyze_block(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Val:
            return analyze_val(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Var:
            return analyze_var(block, walker, entryPoint);
        case lyric_schema::LyricAstId::Def:
            return analyze_def(block, walker, entryPoint);

        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::Init:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:

        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid node");
}
