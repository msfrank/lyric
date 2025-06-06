
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/type_spec.h>
#include <lyric_typing/typing_result.h>

static tempo_utils::Result<lyric_typing::TypeSpec>
parse_s_or_p_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId typeId{};
    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, typeId);
    if (status.notOk())
        return status;

    switch (typeId) {
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
            break;
        default:
            tracer->throwTypingInvariant("failed to parse type; unexpected parser node");
    }

    lyric_common::SymbolPath symbolPath;
    status = walker.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    if (status.notOk())
        return status;

    std::vector<lyric_typing::TypeSpec> typeParameters;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), tracer);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        typeParameters.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_typing::TypeSpec::forSingular(symbolPath, typeParameters);
}

static tempo_utils::Result<lyric_typing::TypeSpec>
parse_i_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstITypeClass))
        tracer->throwTypingInvariant("failed to parse type; unexpected parser node");
    if (walker.numChildren() == 0)
        return tracer->logAndContinue(walker.getLocation(),
            lyric_typing::TypingCondition::kTypeError,
            tempo_tracing::LogSeverity::kError,
            "error parsing type; intersection must contain at least one member");

    std::vector<lyric_typing::TypeSpec> intersectionMembers;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), tracer);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        intersectionMembers.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_typing::TypeSpec::forIntersection(intersectionMembers);
}

static tempo_utils::Result<lyric_typing::TypeSpec>
parse_u_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstUTypeClass))
        tracer->throwTypingInvariant("failed to parse type; unexpected parser node");
    if (walker.numChildren() == 0)
        return tracer->logAndContinue(walker.getLocation(),
            lyric_typing::TypingCondition::kTypeError,
            tempo_tracing::LogSeverity::kError,
            "error parsing type; union must contain at least one member");

    std::vector<lyric_typing::TypeSpec> unionMembers;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), tracer);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        unionMembers.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_typing::TypeSpec::forUnion(unionMembers);
}

tempo_utils::Result<lyric_typing::TypeSpec>
lyric_typing::parse_assignable(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId typeId{};
    TU_RETURN_IF_NOT_OK (walker.parseId(lyric_schema::kLyricAstVocabulary, typeId));

    switch (typeId) {
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
            return parse_s_or_p_type(block, walker, tracer);
        case lyric_schema::LyricAstId::IType:
            return parse_i_type(block, walker, tracer);
        case lyric_schema::LyricAstId::UType:
            return parse_u_type(block, walker, tracer);
        case lyric_schema::LyricAstId::XType:
            return TypeSpec::noReturn();
        default:
            break;
    }

    tracer->throwTypingInvariant("failed to parse type; unexpected parser node");
}

tempo_utils::Result<std::vector<lyric_typing::TypeSpec>>
lyric_typing::parse_type_arguments(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId typeId{};
    TU_RETURN_IF_NOT_OK (walker.parseId(lyric_schema::kLyricAstVocabulary, typeId));

    if (typeId != lyric_schema::LyricAstId::TypeArguments)
        tracer->throwTypingInvariant("failed to parse type arguments; unexpected parser node");

    std::vector<TypeSpec> typeArgumentsSpec;
    for (tu_uint32 i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        TypeSpec argumentSpec;
        TU_ASSIGN_OR_RETURN (argumentSpec, parse_assignable(block, child, tracer));
        typeArgumentsSpec.push_back(argumentSpec);
    }

    return typeArgumentsSpec;
}
