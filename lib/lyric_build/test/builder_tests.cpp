#include <gtest/gtest.h>

#include <lyric_build/lyric_builder.h>
#include <tempo_utils/logging.h>

TEST(LyricBuilder, ComputeSingleTestTarget)
{
    lyric_build::ConfigStore config;
    lyric_build::LyricBuilder builder(config);

    auto configureStatus = builder.configure();
    TU_ASSERT (configureStatus.isOk());

    auto key = lyric_build::TaskKey("test", "target1");
    auto computeTargetResult = builder.computeTarget(key, {}, {}, {});
    TU_LOG_INFO << key << "=" << computeTargetResult;
    ASSERT_TRUE (computeTargetResult.isResult());
}

TEST(LyricBuilder, ComputeSingleTestTargetFails)
{
    auto key = lyric_build::TaskKey("test", "target1");

    lyric_build::ConfigStore config("", "", "", {}, {}, {
        {key, {
                  {"shouldFail", QVariant(true)}
              }}
    });
    lyric_build::LyricBuilder builder(config);

    auto configureStatus = builder.configure();
    TU_ASSERT (configureStatus.isOk());

    auto computeTargetResult = builder.computeTarget(key, {}, {}, {});
    TU_LOG_INFO << key << "=" << computeTargetResult;
    ASSERT_TRUE (computeTargetResult.isResult());
    auto state = computeTargetResult.getResult();
    ASSERT_TRUE (state.getStatus() == lyric_build::TaskState::Status::FAILED);
}

TEST(LyricBuilder, ComputeTestTargetWithDependencies)
{
    auto key = lyric_build::TaskKey("test", "target1");
    auto dep1 = lyric_build::TaskKey("test", "dep1");
    auto dep2 = lyric_build::TaskKey("test", "dep2");
    auto dep3 = lyric_build::TaskKey("test", "dep3");

    lyric_build::ConfigStore config("", "", "", {}, {}, {
        {key, {
            {"dependencies", QVariantList{dep1.toString(), dep2.toString()}}
        }},
        {dep2, {
            {"dependencies", QVariantList{dep3.toString()}}
        }},
    });
    lyric_build::LyricBuilder builder(config);

    auto configureStatus = builder.configure();
    TU_ASSERT (configureStatus.isOk());

    auto computeTargetResult = builder.computeTarget(key, {}, {}, {});
    TU_LOG_INFO << key << "=" << computeTargetResult;
    ASSERT_TRUE (computeTargetResult.isResult());
    auto state = computeTargetResult.getResult();
    ASSERT_TRUE (state.getStatus() == lyric_build::TaskState::Status::COMPLETED);
}

int
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
