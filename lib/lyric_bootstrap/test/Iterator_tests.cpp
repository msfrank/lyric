#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class IteratorTests : public BaseBootstrapFixture {};

TEST_F(IteratorTests, TestForLoopWithExplicitTargetAndIteratorTypes)
{
    auto result = runModule(R"(

        defclass CountdownIterator {
            var _count: I64
            init(count: I64) {
                this._count = count
            }
            impl Iterator[I64] {
                def Valid(): Bool {
                    this._count > 0
                }
                def Next(): I64 {
                    if this._count > 0 {
                        this._count -= 1
                    }
                    this._count
                }
            }
        }

        var count: I64 = 0
        val it: Iterator[I64] = CountdownIterator{3}
        for n: I64 in it {
            count += 1
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(3))));
}

TEST_F(IteratorTests, TestForLoopWithExplicitTargetTypeAndInferredIteratorType)
{
    auto result = runModule(R"(

        defclass CountdownIterator {
            var _count: I64
            init(count: I64) {
                this._count = count
            }
            impl Iterable[CountdownIterator] {
                alias IterableT using Iterable[1] = I64
                def Iterate(it: CountdownIterator): Iterator[IterableT] {
                    this
                }
            }
            impl Iterator[I64] {
                def Valid(): Bool {
                    this._count > 0
                }
                def Next(): I64 {
                    if this._count > 0 {
                        this._count -= 1
                    }
                    this._count
                }
            }
        }

        var count: I64 = 0
        for n: I64 in CountdownIterator{3} {
            count += 1
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(3))));
}
