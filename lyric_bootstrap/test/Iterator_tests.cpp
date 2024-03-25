#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreIterator, TestForLoopWithExplicitTargetAndIteratorTypes)
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

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(3)))));
}

TEST(CoreIterator, TestForLoopWithExplicitTargetTypeAndInferredIteratorType)
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

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellInt(3)))));
}
