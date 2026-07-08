#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>

TEST(LoadArchetype, LoadTypedVal)
{
    lyric_parser::LyricParser parser({});
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        val x: Int = 1
    )", sourceUrl, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    lyric_parser::ArchetypeState state(sourceUrl);

    auto loadArchetypeResult = state.load(archetype);
    ASSERT_TRUE (loadArchetypeResult.isResult());
    auto *root = loadArchetypeResult.getResult();

    ASSERT_TRUE (root->isClass(lyric_schema::kLyricAstRootClass));
    ASSERT_EQ (0, root->numChildren());

    lyric_parser::ArchetypeNode *entry;
    ASSERT_THAT (root->parseAttr(lyric_parser::kLyricAstEntryOffset, entry), tempo_test::IsOk());
    ASSERT_TRUE (entry->isClass(lyric_schema::kLyricAstEntryClass));
    ASSERT_EQ (1, entry->numChildren());

    auto *block = entry->getChild(0);
    ASSERT_EQ (1, block->numChildren());

    auto *val = block->getChild(0);
    ASSERT_TRUE (val->isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (3, val->numAttrs());
    ASSERT_EQ (1, val->numChildren());

    auto identifierValue = val->getAttrValue(lyric_schema::kLyricAstIdentifierProperty);
    ASSERT_TRUE (identifierValue.isValid());
    ASSERT_EQ ("x", identifierValue.getLiteral().getString());

    lyric_parser::ArchetypeNode *type;
    ASSERT_THAT (val->parseAttr(lyric_parser::kLyricAstTypeOffset, type), tempo_test::IsOk());
    ASSERT_TRUE (type->isClass(lyric_schema::kLyricAstSTypeClass));

    lyric_common::SymbolPath symbolPath;
    ASSERT_THAT (type->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath), tempo_test::IsOk());
    ASSERT_TRUE (symbolPath.isValid());
    ASSERT_EQ ("Int", symbolPath.toString());
}
