#include "analyzers.h"
#include <llvm/IR/Instructions.h>

namespace analysis {

void CallGraphAnalyzer::buildCallGraph(llvm::Module &M) {
    if (GraphBuilt) return;
    Callers.clear();
    Callees.clear();

    for (auto &F : M) {
        if (F.isDeclaration()) continue;
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *call = llvm::dyn_cast<llvm::CallBase>(&I)) {
                    if (auto *callee = call->getCalledFunction()) {
                        Callers[callee].insert(&F);
                        Callees[&F].insert(callee);
                    }
                }
            }
        }
    }
    GraphBuilt = true;
}

static bool isRecursiveDFS(const llvm::Function *start, const llvm::Function *current,
                           const std::unordered_map<const llvm::Function *, std::set<const llvm::Function *>> &callees,
                           std::set<const llvm::Function *> &visited) {
    auto it = callees.find(current);
    if (it == callees.end()) return false;

    for (const auto *next : it->second) {
        if (next == start) return true;
        if (visited.insert(next).second) {
            if (isRecursiveDFS(start, next, callees, visited)) {
                return true;
            }
        }
    }
    return false;
}

void CallGraphAnalyzer::analyze(llvm::Function &F, FeatureVector &features) {
    unsigned callCount = 0;
    unsigned externalCalls = 0;

    for (auto &BB : F) {
        for (auto &I : BB) {
            if (auto *call = llvm::dyn_cast<llvm::CallBase>(&I)) {
                callCount++;
                if (auto *callee = call->getCalledFunction()) {
                    if (callee->isDeclaration()) {
                        externalCalls++;
                    }
                } else {
                    externalCalls++;
                }
            }
        }
    }

    features.call_count = callCount;
    features.external_calls = externalCalls;

    features.call_graph_fan_in = Callers.count(&F) ? static_cast<int>(Callers[&F].size()) : 0;
    features.call_graph_fan_out = Callees.count(&F) ? static_cast<int>(Callees[&F].size()) : 0;

    bool isRecursive = false;
    if (Callees.count(&F)) {
        if (Callees[&F].count(&F) > 0) {
            isRecursive = true;
        } else {
            std::set<const llvm::Function *> visited;
            isRecursive = isRecursiveDFS(&F, &F, Callees, visited);
        }
    }
    features.recursive = isRecursive;
}

} // namespace analysis
