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
 * 6. Use regex or string matching on function and API names to flag 'is_sensitive'.
 * 7. Query CallGraph for fan-in and fan-out metrics.
 */

#include "feature_extraction.h"
#include <vector>

namespace analysis {

std::vector<FeatureVector> extract_features(llvm::Module& M) {
    // Return hardcoded dummy data for testing downstream modules
    std::vector<FeatureVector> features;
    
    FeatureVector fv1;
    fv1.function_name = "login_auth";
    fv1.cyclomatic_complexity = 15;
    fv1.basic_block_count = 24;
    fv1.cfg_density = 1.35;
    fv1.instruction_diversity = 42;
    fv1.is_sensitive = true;
    fv1.call_graph_fan_in = 3;
    fv1.call_graph_fan_out = 5;
    features.push_back(fv1);

    FeatureVector fv2;
    fv2.function_name = "util_print_hello";
    fv2.cyclomatic_complexity = 2;
    fv2.basic_block_count = 3;
    fv2.cfg_density = 1.0;
    fv2.instruction_diversity = 12;
    fv2.is_sensitive = false;
    fv2.call_graph_fan_in = 10;
    fv2.call_graph_fan_out = 1;
    features.push_back(fv2);

    return features;
}

} // namespace analysis
