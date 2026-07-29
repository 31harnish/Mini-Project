#include "analyzers.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CFG.h>
#include <llvm/Analysis/LoopInfo.h>

namespace analysis {

void ComplexityAnalyzer::analyze(llvm::Function &F, llvm::LoopInfo &LI, FeatureVector &features) {
    unsigned nodes = 0;
    unsigned edges = 0;
    unsigned branchCount = 0;
    unsigned switchCount = 0;

    for (auto &BB : F) {
        nodes++;
        for (auto *Succ : llvm::successors(&BB)) {
            (void)Succ;
            edges++;
        }

        for (auto &I : BB) {
            if (llvm::isa<llvm::BranchInst>(&I)) {
                branchCount++;
            } else if (llvm::isa<llvm::SwitchInst>(&I)) {
                switchCount++;
            }
        }
    }

    // Cyclomatic Complexity: E - V + 2
    if (nodes > 0) {
        features.cyclomatic_complexity = edges - nodes + 2;
    } else {
        features.cyclomatic_complexity = 0;
    }

    features.branch_count = branchCount;
    features.switch_count = switchCount;

    // Loop info statistics
    unsigned loopCount = 0;
    unsigned maxLoopDepth = 0;

    // Get all loops
    for (auto *L : LI) {
        loopCount++;
        // Helper lambda to count subloops recursively and get max depth
        auto countLoops = [&](auto& self, const llvm::Loop* lp) -> void {
            unsigned depth = lp->getLoopDepth();
            if (depth > maxLoopDepth) {
                maxLoopDepth = depth;
            }
            for (auto* subL : *lp) {
                loopCount++;
                self(self, subL);
            }
        };
        countLoops(countLoops, L);
    }

    features.loop_count = loopCount;
    features.max_loop_depth = maxLoopDepth;
}

} // namespace analysis
