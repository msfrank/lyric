
#include <lyric_compiler/compiler_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/compiler_schema.h>

lyric_compiler::CompilerChoice::CompilerChoice(
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CompilerChoice::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricCompilerNs))
        return {};
    auto *resource = lyric_schema::kLyricCompilerVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_INFO << "decide CompilerChoice@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();

    switch (astId) {

        case lyric_schema::LyricCompilerId::PopResult: {
            return driver->popResult();
        }

        case lyric_schema::LyricCompilerId::PushResult: {
            auto *typeSystem = driver->getTypeSystem();
            lyric_parser::ArchetypeNode *typeNode;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
            lyric_typing::TypeSpec typeSpec;
            TU_ASSIGN_OR_RETURN (typeSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
            lyric_common::TypeDef typeDef;
            TU_ASSIGN_OR_RETURN (typeDef, typeSystem->resolveAssignable(block, typeSpec));
            return driver->pushResult(typeDef);
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid compiler node");
    }
}