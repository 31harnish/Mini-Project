/**
 * @file feature_extraction.cpp
 * @brief Module 1 — Feature Extraction Engine
 *
 * Responsibility: walk every function in the IR, compute cyclomatic complexity,
 * basic block count, CFG density, instruction diversity, detect sensitive
 * API/function-name patterns (login, encrypt, decrypt, auth, key), compute
 * call graph fan-in/fan-out.
 *
 * Expected LLVM APIs:
 * - llvm::Module, llvm::Function, llvm::BasicBlock, llvm::Instruction
 * - llvm::CallGraph (for fan-in/fan-out)
 * - llvm::LoopInfo (for complexity)
 *
 * Inputs: llvm::Module (parsed LLVM IR)
 * Outputs: std::vector<FeatureVector>
 *
 * TODOs for Real Implementation:
 * 1. Initialize LLVM analysis passes for CallGraph and LoopInfo.
 * 2. Iterate over all functions in the llvm::Module.
 * 3. Calculate cyclomatic complexity (edges - nodes + 2).
 * 4. Count basic blocks and unique instruction opcodes.
 * 5. Calculate CFG density (edges / nodes).
 * 6. Use regex or string matching on function and API names to flag
 * 'is_sensitive'.
 * 7. Query CallGraph for fan-in and fan-out metrics.
 */

#include "feature_extraction.h"
#include <algorithm>
#include <cctype>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <set>
#include <unordered_map>
#include <vector>

namespace analysis {

static std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

static bool is_sensitive_name(const std::string &name) {
  std::string lower = to_lower(name);
  return lower.find("login") != std::string::npos ||
         lower.find("encrypt") != std::string::npos ||
         lower.find("decrypt") != std::string::npos ||
         lower.find("auth") != std::string::npos ||
         lower.find("key") != std::string::npos ||
         lower.find("cipher") != std::string::npos ||
         lower.find("crypto") != std::string::npos;
}

std::vector<FeatureVector> extract_features(llvm::Module &M) {
  std::vector<FeatureVector> features;

  // First pass: build call graph caller/callee sets to compute fan-in/fan-out
  std::unordered_map<const llvm::Function *, std::set<const llvm::Function *>>
      callers;
  std::unordered_map<const llvm::Function *, std::set<const llvm::Function *>>
      callees;

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *call = llvm::dyn_cast<llvm::CallBase>(&I)) {
          if (auto *callee = call->getCalledFunction()) {
            callers[callee].insert(&F);
            callees[&F].insert(callee);
          }
        }
      }
    }
  }

  // Second pass: extract features for each function
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    FeatureVector fv;
    fv.function_name = F.getName().str();

    // Basic Block count
    fv.basic_block_count = static_cast<int>(F.size());

    // Instruction diversity and total complexity
    std::set<unsigned> unique_opcodes;
    int edge_count = 0;

    for (auto &BB : F) {
      edge_count += static_cast<int>(BB.getTerminator()->getNumSuccessors());
      for (auto &I : BB) {
        unique_opcodes.insert(I.getOpcode());
      }
    }

    fv.instruction_diversity = static_cast<int>(unique_opcodes.size());

    // Cyclomatic complexity: E - V + 2
    fv.cyclomatic_complexity = edge_count - fv.basic_block_count + 2;
    if (fv.basic_block_count == 0) {
      fv.cyclomatic_complexity = 0;
    }

    // CFG density: E / V
    if (fv.basic_block_count > 0) {
      fv.cfg_density = static_cast<double>(edge_count) / fv.basic_block_count;
    } else {
      fv.cfg_density = 0.0;
    }

    // Sensitivity check (function name or any of its called functions name)
    bool sensitive = is_sensitive_name(fv.function_name);
    if (!sensitive && callees.count(&F)) {
      for (const auto *callee : callees[&F]) {
        if (is_sensitive_name(callee->getName().str())) {
          sensitive = true;
          break;
        }
      }
    }
    fv.is_sensitive = sensitive;

    // Call Graph Fan-in / Fan-out
    fv.call_graph_fan_in =
        callers.count(&F) ? static_cast<int>(callers[&F].size()) : 0;
    fv.call_graph_fan_out =
        callees.count(&F) ? static_cast<int>(callees[&F].size()) : 0;

    features.push_back(fv);
  }

  return features;
}

} // namespace analysis
