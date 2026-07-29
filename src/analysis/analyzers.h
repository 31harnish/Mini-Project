#ifndef ANALYZERS_H
#define ANALYZERS_H

#include "../../include/interfaces.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/LoopInfo.h>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>

namespace analysis {

class CFGAnalyzer {
public:
    void analyze(llvm::Function &F, FeatureVector &features);
};

class ComplexityAnalyzer {
public:
    void analyze(llvm::Function &F, llvm::LoopInfo &LI, FeatureVector &features);
};

class InstructionAnalyzer {
public:
    void analyze(llvm::Function &F, FeatureVector &features);
};

class SecurityAnalyzer {
private:
    std::unordered_map<std::string, std::vector<std::string>> Database;
    bool Loaded = false;
    void loadDatabase();

public:
    SecurityAnalyzer();
    void analyze(llvm::Function &F, FeatureVector &features);
};

class ConstantAnalyzer {
public:
    void analyze(llvm::Function &F, FeatureVector &features);
};

class CallGraphAnalyzer {
private:
    std::unordered_map<const llvm::Function *, std::set<const llvm::Function *>> Callers;
    std::unordered_map<const llvm::Function *, std::set<const llvm::Function *>> Callees;
    bool GraphBuilt = false;

public:
    void buildCallGraph(llvm::Module &M);
    void analyze(llvm::Function &F, FeatureVector &features);
};

class FeatureSerializer {
public:
    static void serialize(const std::vector<FeatureVector> &featuresList, const std::string &outputPath);
};

} // namespace analysis

#endif // ANALYZERS_H
