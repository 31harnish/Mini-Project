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
 */

#include "build_controller.h"
#include "../analysis/feature_extraction.h"
#include "../analysis/threat_analyzer.h"
#include "../orchestration/module4_orchestrator.h"
#include "../orchestration/real_backend.h"
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

void run_full_pipeline(llvm::Module &M,
                       const std::string &ir_in_path,
                       const std::string &ir_out_path,
                       const std::string &opt_path,
                       const std::string &plugin_path) {
  std::cout << "Starting full obfuscation pipeline (Modules 1-4)..."
            << std::endl;

  // ---- Modules 1-3: Analysis + Planning ----
  auto features = analysis::extract_features(M);
  auto threats = analysis::analyze_threats(features);
  auto plans = planner::generate_plan(threats, features);

  std::cout << "========================================================\n"
               "  MODULE 3 : Protection Planner\n"
               "========================================================"
            << std::endl;
  for (const auto &plan : plans) {
    std::cout << dumpPlanToJson(plan) << std::endl;
  }
  std::cout << "-----------------------------------" << std::endl;

  // ---- Module 4: Orchestration via RealBackend ----
  std::cout << "========================================================\n"
               "  MODULE 4 : Orchestration Layer\n"
               "========================================================"
            << std::endl;

  orchestration::RealBackend backend(opt_path, plugin_path);

  // The orchestrator works on files: the first plan transforms ir_in → ir_out,
  // then each subsequent plan transforms ir_out → ir_out (in-place chain).
  std::string current_input = ir_in_path;

  for (size_t i = 0; i < plans.size(); ++i) {
    const auto &plan = plans[i];

    // Skip functions with no passes selected.
    if (plan.selected_passes.empty()) {
      std::cout << "  [BuildController] Skipping '" << plan.function_name
                << "' — no passes selected." << std::endl;
      continue;
    }

    std::cout << "  [BuildController] Applying " << plan.selected_passes.size()
              << " passes to '" << plan.function_name
              << "' at intensity " << plan.intensity_level
              << " (seed=" << plan.random_seed << ")" << std::endl;

    try {
      auto metrics = orchestration::RunModule4(
          backend,
          plan.function_name,
          plan.selected_passes,
          plan.intensity_level,
          plan.random_seed,
          current_input,
          ir_out_path);

      std::cout << "  [BuildController] Result: overhead=" << metrics.overhead
                << " size_bloat=" << metrics.size_bloat
                << " deterministic=" << (metrics.fully_deterministic ? "yes" : "no")
                << std::endl;

      // After the first successful run, subsequent plans chain from the output.
      current_input = ir_out_path;
    } catch (const std::exception &e) {
      std::cerr << "  [BuildController] ERROR on '" << plan.function_name
                << "': " << e.what() << std::endl;
      // Continue with remaining functions rather than aborting the pipeline.
    }
  }

  std::cout << "========================================================\n"
               "  Pipeline complete. Output: " << ir_out_path << "\n"
               "========================================================"
            << std::endl;
}

} // namespace planner

