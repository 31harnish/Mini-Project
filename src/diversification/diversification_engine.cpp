/**
 * @file diversification_engine.cpp
 * @brief Module 6 — Diversification Engine
 * 
 * Responsibility: ensure repeated compilations of the same source produce
 * structurally different binaries — randomize pass order, vary which decoy
 * templates get used, vary CFG dispatcher style.
 * 
 * Inputs: ProtectionPlan, a random seed
 * Outputs: a randomized execution order/config for Module 4 and Module 5's passes
 */

#include "diversification_engine.h"
#include <algorithm>
#include <random>
#include <functional>
#include <iostream>

namespace diversification {

int derive_function_seed(const std::string& function_name, int global_seed) {
    std::hash<std::string> hasher;
    size_t name_hash = hasher(function_name);
    // Combine hash with global seed
    return static_cast<int>(name_hash ^ (static_cast<size_t>(global_seed) * 0x9e3779b9 + (name_hash << 6) + (name_hash >> 2)));
}

std::vector<std::string> get_randomized_pass_order(const ProtectionPlan& plan, int random_seed) {
    std::vector<std::string> passes = plan.selected_passes;
    if (passes.empty()) {
        return passes;
    }

    int seed = derive_function_seed(plan.function_name, random_seed);
    std::mt19937 g(seed);
    std::shuffle(passes.begin(), passes.end(), g);
    return passes;
}

FunctionDiversificationPlan generate_diversified_config(const ProtectionPlan& plan, int global_seed) {
    FunctionDiversificationPlan div_plan;
    div_plan.function_name = plan.function_name;
    div_plan.function_seed = derive_function_seed(plan.function_name, global_seed);
    div_plan.randomized_passes = get_randomized_pass_order(plan, global_seed);

    std::mt19937 rng(div_plan.function_seed);

    const std::vector<std::string> decoy_templates = {
        "crypto_sha256_mock",
        "auth_validation_decoy",
        "pointer_arithmetic_divergence",
        "dead_code_recursion_decoy",
        "fake_key_expansion_loop"
    };

    const std::vector<DispatcherStyle> dispatcher_styles = {
        DispatcherStyle::SWITCH_CASE,
        DispatcherStyle::INDIRECT_JUMP_TABLE,
        DispatcherStyle::NESTED_IF_ELSE,
        DispatcherStyle::OPAQUE_PREDICATE_CHAINS
    };

    for (const auto& pass_name : div_plan.randomized_passes) {
        DiversifiedPassConfig pass_cfg;
        pass_cfg.pass_name = pass_name;
        pass_cfg.seed = static_cast<int>(rng());
        
        std::uniform_int_distribution<size_t> dist_style(0, dispatcher_styles.size() - 1);
        pass_cfg.dispatcher_style = dispatcher_styles[dist_style(rng)];

        // Bogus block density scales with plan intensity (level 1-5) + random jitter (+/- 10%)
        std::uniform_int_distribution<int> jitter_dist(-10, 10);
        int base_density = plan.intensity_level * 15; // e.g. level 3 -> 45%
        pass_cfg.bogus_block_density = std::clamp(base_density + jitter_dist(rng), 10, 95);

        std::uniform_int_distribution<size_t> dist_decoy(0, decoy_templates.size() - 1);
        pass_cfg.decoy_template_type = decoy_templates[dist_decoy(rng)];

        div_plan.pass_configs.push_back(pass_cfg);
    }

    return div_plan;
}

} // namespace diversification
