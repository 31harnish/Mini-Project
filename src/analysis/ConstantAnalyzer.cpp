#include "analyzers.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <set>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>

namespace analysis {

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

static double calculateShannonEntropy(const std::string &s) {
    if (s.empty()) return 0.0;
    std::map<char, double> frequencies;
    for (char c : s) {
        frequencies[c]++;
    }
    double entropy = 0.0;
    double len = s.length();
    for (auto const& kv : frequencies) {
        double p = kv.second / len;
        entropy -= p * std::log2(p);
    }
    return entropy;
}

static const llvm::GlobalVariable* getGlobalVariable(const llvm::Value *val) {
    if (auto *gv = llvm::dyn_cast<llvm::GlobalVariable>(val)) {
        return gv;
    }
    if (auto *ce = llvm::dyn_cast<llvm::ConstantExpr>(val)) {
        for (unsigned i = 0; i < ce->getNumOperands(); ++i) {
            if (auto *gv = getGlobalVariable(ce->getOperand(i))) {
                return gv;
            }
        }
    }
    return nullptr;
}

void ConstantAnalyzer::analyze(llvm::Function &F, FeatureVector &features) {
    unsigned secretStrings = 0;
    unsigned integerConstants = 0;
    double maxEntropy = 0.0;
    std::set<const llvm::GlobalVariable*> checkedGlobals;

    std::vector<std::string> sensitiveKeywords = {
        "password", "secret", "token", "apikey", "license",
        "begin rsa", "private key", "cipher", "crypto"
    };

    for (auto &BB : F) {
        for (auto &I : BB) {
            // Scan instruction operands
            for (unsigned opIdx = 0; opIdx < I.getNumOperands(); ++opIdx) {
                llvm::Value *op = I.getOperand(opIdx);
                if (llvm::isa<llvm::ConstantInt>(op)) {
                    integerConstants++;
                }

                // Trace global variable references
                if (const llvm::GlobalVariable *gv = getGlobalVariable(op)) {
                    if (checkedGlobals.insert(gv).second) {
                        if (gv->hasInitializer()) {
                            if (auto *cdata = llvm::dyn_cast<llvm::ConstantDataSequential>(gv->getInitializer())) {
                                if (cdata->isString()) {
                                    std::string rawStr = cdata->getAsString().str();
                                    double entropy = calculateShannonEntropy(rawStr);
                                    if (entropy > maxEntropy) {
                                        maxEntropy = entropy;
                                    }

                                    std::string strContent = toLower(rawStr);
                                    bool isSecret = false;
                                    for (const auto& keyword : sensitiveKeywords) {
                                        if (strContent.find(keyword) != std::string::npos) {
                                            isSecret = true;
                                            break;
                                        }
                                    }
                                    if (isSecret) {
                                        secretStrings++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    features.secret_strings = secretStrings;
    features.integer_constants = integerConstants;
    features.string_entropy = maxEntropy;
}

} // namespace analysis
