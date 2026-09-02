/**
 * @file protection_planner.cpp
 * @brief Module 3 — Protection Planner
 */

#include "protection_planner.h"
#include <algorithm>
#include <map>

namespace planner {

// Ordering definition
int getPassPriority(const std::string& pass) {
    if (pass == "string_obfuscation" || pass == "instruction_substitution" || pass == "semantic_divergence") return 1;
    if (pass == "decoy_idiom" || pass == "bogus_control_flow") return 2;
    if (pass == "control_flow_flattening" || pass == "adaptive_cfg_diversification") return 3;
    return 99;
}

std::vector<ProtectionPlan> generate_plan(
    const std::vector<ThreatReport>& reports,
    const std::vector<FeatureVector>& features) 
{
    std::vector<ProtectionPlan> plans;
    
    // Create lookup table
    std::map<std::string, FeatureVector> feature_map;
    for (const auto& f : features) {
        feature_map[f.function_name] = f;
    }

    for (const auto& report : reports) {
        // Fallback to a default FeatureVector if not found, though realistically it should be
        FeatureVector feat = feature_map[report.function_name];
        ProtectionPlan plan;
        plan.function_name = report.function_name;

        // 1. Calculate Intensity
        int intensity = 1;
        if (report.risk_score >= 0.9) intensity = 5;
        else if (report.risk_score >= 0.7) intensity = 4;
        else if (report.risk_score >= 0.5) intensity = 3;
        else if (report.risk_score >= 0.2) intensity = 2;

        // 2. Budget Overhead Cap
        if (feat.basic_block_count > 500 && feat.cyclomatic_complexity > 50 && !feat.is_sensitive) {
            intensity = std::min(intensity, 4);
        }
        plan.intensity_level = intensity;

        // 3. Rounds and Target Accuracy
        plan.max_transformation_rounds = (intensity >= 5) ? 3 : ((intensity >= 3) ? 2 : 1);
        plan.target_llm_reconstruction_accuracy = 1.0 - (intensity * 0.15);

        // 4. Pass Selection
        std::vector<std::string> passes;
        if (intensity == 1) {
            passes = {"instruction_substitution"};
        } else if (intensity == 2) {
            passes = {"instruction_substitution", "bogus_control_flow"};
        } else if (intensity == 3) {
            passes = {"bogus_control_flow", "adaptive_cfg_diversification", "semantic_divergence"};
        } else if (intensity == 4) {
            passes = {"decoy_idiom", "bogus_control_flow", "control_flow_flattening"};
        } else {
            passes = {"string_obfuscation", "decoy_idiom", "control_flow_flattening", "semantic_divergence"};
        }

        // 5. Sensitivity Overrides
        if (report.sensitivity_category == "authentication") {
            if (std::find(passes.begin(), passes.end(), "string_obfuscation") == passes.end()) {
                passes.push_back("string_obfuscation");
            }
            if (std::find(passes.begin(), passes.end(), "decoy_idiom") == passes.end()) {
                passes.push_back("decoy_idiom");
            }
        } else if (report.sensitivity_category == "encryption" && feat.basic_block_count > 100) {
            // Remove heavy flattening to preserve perf
            passes.erase(std::remove(passes.begin(), passes.end(), "control_flow_flattening"), passes.end());
            if (std::find(passes.begin(), passes.end(), "semantic_divergence") == passes.end()) {
                passes.push_back("semantic_divergence");
            }
            if (std::find(passes.begin(), passes.end(), "instruction_substitution") == passes.end()) {
                passes.push_back("instruction_substitution");
            }
        }

        // 6. Sort Passes for logical application order
        std::sort(passes.begin(), passes.end(), [](const std::string& a, const std::string& b) {
            return getPassPriority(a) < getPassPriority(b);
        });

        // Remove duplicates
        passes.erase(std::unique(passes.begin(), passes.end()), passes.end());
        plan.selected_passes = passes;

        plans.push_back(plan);
    }
    return plans;
}

} // namespace planner
