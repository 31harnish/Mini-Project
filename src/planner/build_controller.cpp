/**
 * @file build_controller.cpp
 * @brief Module 7 — Adaptive Evaluation & Build Controller
 * 
 * Responsibility: orchestrate the full pipeline in order,
 * and implement the iterate loop — after each transformation round, check
 * intermediate metrics against the plan's target and either run another round
 * or finalize the build.
 * 
 * Expected LLVM APIs:
 * - llvm::Module, llvm::PassManager (if integrated as a proper pass plugin)
 * 
 * Inputs: llvm::Module, all module function pointers/interfaces
 * Outputs: fully protected llvm::Module ready for codegen
 * 
 * TODOs for Real Implementation:
 * 1. Call Module 1 to get FeatureVectors.
 * 2. Call Module 2 to get ThreatReports.
 * 3. Call Module 3 to generate ProtectionPlans.
 * 4. Implement a loop bounded by `max_transformation_rounds`.
 * 5. Call Module 6 for randomized ordering, then Modules 4 and 5 to apply passes.
 * 6. (Optional) Run intermediate evaluation via Module 7 to check if targets are met early.
 */

#include "build_controller.h"
#include "../analysis/feature_extraction.h"
#include "../analysis/threat_analyzer.h"
#include "protection_planner.h"
#include "planner_utils.h"
#include "../diversification/diversification_engine.h"
#include "../passes/existing/ollvm_passes.h"
#include "../passes/novel/ai_resistant_passes.h"
#include "../eval/evaluation_engine.h"
#include <llvm/IR/Verifier.h>
#include <iostream>

namespace planner {

void run_obfuscation_pipeline(llvm::Module& M) {
    std::cout << "Starting obfuscation pipeline..." << std::endl;
    
    // 1. Feature Extraction
    auto features = analysis::extract_features(M);
    
    // 2. Threat Analysis
    auto threats = analysis::analyze_threats(features);
    
    // 3. Protection Planning
    auto plans = planner::generate_plan(threats, features);
    std::cout << "--- Planner Output (JSON Debug) ---" << std::endl;
    for (const auto& plan : plans) {
        std::cout << dumpPlanToJson(plan) << std::endl;
    }
    std::cout << "-----------------------------------" << std::endl;

    
    // 4, 5, 6, 7. Iterative Application with LLM-Confusion Feedback Loop
    for (const auto& plan : plans) {
        int rounds = plan.max_transformation_rounds;
        
        llvm::Function* F = M.getFunction(plan.function_name);
        if (!F || F->isDeclaration()) {
            std::cerr << "Warning: Function " << plan.function_name << " not found or is a declaration. Skipping." << std::endl;
            continue;
        }

        std::cout << "Starting protection for " << plan.function_name << " (Target LLM accuracy <= " 
                  << (plan.target_llm_reconstruction_accuracy * 100) << "%)" << std::endl;
                  
        for (int i = 0; i < rounds; ++i) {
            // Get randomized order
            auto ordered_passes = diversification::get_randomized_pass_order(plan, 42 + i);
            
            std::cout << "  Applying passes (Round " << i + 1 << ")" << std::endl;
            
            // Execute Module 4 (Existing OLLVM passes)
            passes::existing::apply_ollvm_passes(*F, plan);
            
            // Verify IR correctness
            if (llvm::verifyFunction(*F, &llvm::errs())) {
                std::cerr << "Error: IR verification failed for " << plan.function_name << " after obfuscation!" << std::endl;
                // In a real framework we might abort or rollback here
            }

            // passes::novel::apply_ai_resistant_passes(*F, plan);
            
            // 7. Intermediate Evaluation
            // Evaluate intermediate IR or a mock binary to check LLM reconstruction accuracy
            auto metrics = eval::evaluate_binary("out.base.bin", "out.intermediate.bin");
            
            std::cout << "  -> Current LLM Reconstruction Accuracy: " << (metrics.llm_reconstruction_accuracy * 100) << "%" << std::endl;
            
            if (metrics.llm_reconstruction_accuracy <= plan.target_llm_reconstruction_accuracy) {
                std::cout << "  -> SUCCESS: LLM confusion budget met early. Stopping iterations for " << plan.function_name << "." << std::endl;
                break;
            }
        }
    }
    
    // Final evaluation
    auto final_metrics = eval::evaluate_binary("out.base.bin", "out.obf.bin");
    std::cout << "Pipeline complete. Final runtime overhead: " << final_metrics.runtime_overhead_pct << "%" << std::endl;
}

} // namespace planner
