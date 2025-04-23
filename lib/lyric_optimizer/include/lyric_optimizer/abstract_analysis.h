#ifndef LYRIC_OPTIMIZER_ABSTRACT_ANALYSIS_H
#define LYRIC_OPTIMIZER_ABSTRACT_ANALYSIS_H

#include "basic_block.h"

namespace lyric_optimizer {

    template<class T>
    class AbstractAnalysis {
    public:
        virtual ~AbstractAnalysis() = default;

        virtual T&& initialize(const BasicBlock &basicBlock) = 0;

        virtual T&& meet(std::vector<T> &predecessors) = 0;

        virtual T&& transfer(const BasicBlock &basicBlock, T &prev) = 0;
    };

}

#endif // LYRIC_OPTIMIZER_ABSTRACT_ANALYSIS_H
