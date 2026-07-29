#include "analyzers.h"
#include <llvm/IR/CFG.h>

namespace analysis {

void CFGAnalyzer::analyze(llvm::Function &F, FeatureVector &features) {
    unsigned nodes = 0;
    unsigned edges = 0;

    for (auto &BB : F) {
        nodes++;
        // Count successor edges
        for (auto *Succ : llvm::successors(&BB)) {
            (void)Succ; // Avoid unused variable warnings
            edges++;
        }
    }

    features.basic_block_count = nodes;
    features.cfg_density = (nodes > 0) ? static_cast<double>(edges) / nodes : 0.0;
}

} // namespace analysis
