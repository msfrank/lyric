#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
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

    ASSERT_TRUE (root->isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (1, root->numChildren());

    auto *val = root->getChild(0);
    ASSERT_TRUE (val->isClass(lyric_schema::kLyricAstValClass));
    ASSERT_EQ (3, val->numAttrs());
    ASSERT_EQ (1, val->numChildren());

    auto identifierValue = val->getAttrValue(lyric_schema::kLyricAstIdentifierProperty);
    ASSERT_TRUE (identifierValue.isValid());
    ASSERT_EQ ("x", identifierValue.getLiteral().getString());

    auto typeValue = val->getAttrValue(lyric_schema::kLyricAstTypeOffsetProperty);
    ASSERT_TRUE (typeValue.isValid());
    auto *type = typeValue.getNode();
    ASSERT_TRUE (type->isClass(lyric_schema::kLyricAstSTypeClass));

    auto symbolPathValue = type->getAttrValue(lyric_schema::kLyricAstSymbolPathProperty);
    ASSERT_TRUE (symbolPathValue.isValid());
    ASSERT_EQ ("Int", symbolPathValue.getLiteral().getString());
}
