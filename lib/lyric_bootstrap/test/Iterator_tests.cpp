#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class IteratorTests : public BaseBootstrapFixture {};

TEST_F(IteratorTests, TestForLoopWithExplicitTargetAndIteratorTypes)
{
    auto result = runModule(R"(

        defclass CountdownIterator {
            var _count: Int
            init(count: Int) {
                set this._count = count
            }
            impl Iterator[Int] {
                def Valid(): Bool {
                    this._count > 0
                }
                def Next(): Int {
                    if this._count > 0 {
                        set this._count -= 1
                    }
                    this._count
                }
            }
        }

        var count: Int = 0
        val it: Iterator[Int] = CountdownIterator{3}
        for n: Int in it {
            set count += 1
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(3))));
}

TEST_F(IteratorTests, TestForLoopWithExplicitTargetTypeAndInferredIteratorType)
{
    auto result = runModule(R"(

        defclass CountdownIterator {
            var _count: Int
            init(count: Int) {
                set this._count = count
            }
            impl Iterator[Int] {
                def Valid(): Bool {
                    this._count > 0
                }
                def Next(): Int {
                    if this._count > 0 {
                        set this._count -= 1
                    }
                    this._count
                }
            }
        }

        var count: Int = 0
        for n: Int in CountdownIterator{3} {
            set count += 1
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(3))));
}
