/**
 * @file protection_planner.cpp
 * @brief Module 3 — Protection Planner
 * 
 * Responsibility: rule-based decision logic mapping risk score + complexity to
 * a selected set of passes, intensity, and round budget per function. Document
 * the actual rule thresholds as constants at the top of the file so they're
 * easy to tune later.
 * 
 * Expected LLVM APIs: None strictly required here.
 * 
 * Inputs: std::vector<ThreatReport>, std::vector<FeatureVector>
 * Outputs: std::vector<ProtectionPlan>
 * 
 * TODOs for Real Implementation:
 * 1. Define rule thresholds (e.g. HIGH_RISK_THRESHOLD = 0.8).
 * 2. Cross-reference ThreatReport and FeatureVector by function_name.
 * 3. Assign heavy protection (e.g. all passes, high intensity) to high-risk functions.
 * 4. Assign lighter or decoy-only protection to lower-risk functions to save overhead.
 */

#include "protection_planner.h"
#include <algorithm>

namespace planner {

// Tuning Constants
const double HIGH_RISK_THRESHOLD = 0.8;
const double MED_RISK_THRESHOLD = 0.4;

std::vector<ProtectionPlan> generate_plan(
    const std::vector<ThreatReport>& reports,
    const std::vector<FeatureVector>& features) {
    
    std::vector<ProtectionPlan> plans;
    
    for (const auto& report : reports) {
        ProtectionPlan plan;
        plan.function_name = report.function_name;
        
        if (report.risk_score >= HIGH_RISK_THRESHOLD) {
            plan.selected_passes = {"ControlFlowFlattening", "DecoyIdiomInjection", "SemanticDivergence", "SymbolPoisoning"};
            plan.intensity_level = 5;
            plan.max_transformation_rounds = 3;
            plan.target_llm_reconstruction_accuracy = 0.20; // Need high confusion (<= 20% accuracy)
        } else {
            plan.selected_passes = {"InstructionSubstitution", "SymbolPoisoning"};
            plan.intensity_level = 2;
            plan.max_transformation_rounds = 1;
            plan.target_llm_reconstruction_accuracy = 0.50; // Moderate confusion is fine
        }
        plans.push_back(plan);
    }
    
    return plans;
}

} // namespace planner
