#ifndef FEATURE_EXTRACTION_H
#define FEATURE_EXTRACTION_H

#include "../../include/interfaces.h"
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Module.h>
#include <vector>

namespace analysis {

// Legacy API called by build_controller.cpp
std::vector<FeatureVector> extract_features(llvm::Module& M);

// Proper LLVM Module Pass
class FeatureExtractionPass : public llvm::PassInfoMixin<FeatureExtractionPass> {
public:
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &AM);
};

} // namespace analysis

#endif // FEATURE_EXTRACTION_H
