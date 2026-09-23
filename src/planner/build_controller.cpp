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
 * 5. Call Module 6 for randomized ordering, then Modules 4 and 5 to apply
 * passes.
 * 6. (Optional) Run intermediate evaluation via Module 7 to check if targets
 * are met early.
 */

#include "build_controller.h"
#include "../analysis/feature_extraction.h"
#include "../analysis/threat_analyzer.h"
#include "planner_utils.h"
#include "protection_planner.h"
#include <iostream>
#include <llvm/IR/Verifier.h>

namespace planner {

void run_obfuscation_pipeline(llvm::Module &M) {
  std::cout << "Starting obfuscation pipeline..." << std::endl;

  // 1. Feature Extraction
  auto features = analysis::extract_features(M);

  // 2. Threat Analysis
  auto threats = analysis::analyze_threats(features);

  // 3. Protection Planning
  auto plans = planner::generate_plan(threats, features);
  std::cout << "========================================================\n"
               "  MODULE 3 : Protection Planner\n"
               "========================================================"
            << std::endl;
  for (const auto &plan : plans) {
    std::cout << dumpPlanToJson(plan) << std::endl;
  }
  std::cout << "-----------------------------------" << std::endl;
}

} // namespace planner
