#include "feature_extraction.h"
#include "analyzers.h"
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Function.h>
#include <vector>

namespace analysis {

static std::vector<FeatureVector> runFeatureExtraction(llvm::Module &M, bool serialize) {
    std::vector<FeatureVector> featuresList;

    // 1. Build the Call Graph
    CallGraphAnalyzer callGraphAnalyzer;
    callGraphAnalyzer.buildCallGraph(M);

    // 2. Initialize the remaining analyzers
    CFGAnalyzer cfgAnalyzer;
    ComplexityAnalyzer complexityAnalyzer;
    InstructionAnalyzer instAnalyzer;
    SecurityAnalyzer securityAnalyzer;
    ConstantAnalyzer constantAnalyzer;

    for (auto &F : M) {
        if (F.isDeclaration()) continue;

        FeatureVector fv;
        fv.function_name = F.getName().str();

        // 3. Compute DominatorTree and LoopInfo manually to remain version-agnostic
        llvm::DominatorTree DT(F);
        llvm::LoopInfo LI(DT);

        // 4. Run all analyses on this function
        cfgAnalyzer.analyze(F, fv);
        complexityAnalyzer.analyze(F, LI, fv);
        instAnalyzer.analyze(F, fv);
        securityAnalyzer.analyze(F, fv);
        constantAnalyzer.analyze(F, fv);
        callGraphAnalyzer.analyze(F, fv);

        featuresList.push_back(fv);
    }

    // 5. Output features.json
    if (serialize) {
        FeatureSerializer::serialize(featuresList, "features.json");
    }

    return featuresList;
}

std::vector<FeatureVector> extract_features(llvm::Module &M) {
    return runFeatureExtraction(M, true);
}

llvm::PreservedAnalyses FeatureExtractionPass::run(llvm::Module &M, llvm::ModuleAnalysisManager &AM) {
    (void)AM;
    runFeatureExtraction(M, true);
    return llvm::PreservedAnalyses::all();
}

} // namespace analysis
