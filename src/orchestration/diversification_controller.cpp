#include "orchestration/diversification_controller.h"
#include <algorithm>
#include <random>
#include <cassert>

namespace orchestration {

static void EnforceDependencyConstraints(std::vector<std::string>& passes) {
    auto sub_it = std::find(passes.begin(), passes.end(), "instruction_substitution");
    auto flat_it = std::find(passes.begin(), passes.end(), "control_flow_flattening");
    
    // If both exist and substitution is AFTER flattening
    if (sub_it != passes.end() && flat_it != passes.end() && std::distance(passes.begin(), sub_it) > std::distance(passes.begin(), flat_it)) {
        std::string sub_val = *sub_it;
        passes.erase(sub_it);
        
        // Re-find flat_it in case of iterator invalidation or shifting, though erasing after it wouldn't shift it.
        flat_it = std::find(passes.begin(), passes.end(), "control_flow_flattening");
        passes.insert(flat_it, sub_val);
    }

#ifndef NDEBUG
    sub_it = std::find(passes.begin(), passes.end(), "instruction_substitution");
    flat_it = std::find(passes.begin(), passes.end(), "control_flow_flattening");
    if (sub_it != passes.end() && flat_it != passes.end()) {
        assert(std::distance(passes.begin(), sub_it) < std::distance(passes.begin(), flat_it));
    }
#endif
}

std::vector<RoundPlan> GenerateRoundPlans(
    const std::vector<std::string>& selected_passes,
    uint64_t base_seed,
    int max_rounds) 
{
    std::vector<RoundPlan> plans;
    std::vector<std::string> prev_order;

    for (int i = 0; i < max_rounds; ++i) {
        uint64_t round_seed = base_seed ^ (i + 1);
        std::vector<std::string> current_order = selected_passes;

        int retries = 0;
        const int MAX_RETRIES = 5;
        uint64_t actual_seed_used = round_seed;

        while (true) {
            actual_seed_used = round_seed + retries;
            std::mt19937_64 rng(actual_seed_used);
            
            std::shuffle(current_order.begin(), current_order.end(), rng);

            EnforceDependencyConstraints(current_order);

            if (selected_passes.size() <= 1 || prev_order.empty() || current_order != prev_order) {
                break;
            }

            retries++;
            if (retries >= MAX_RETRIES) {
                break; // Fallback to avoid infinite loop
            }
        }

        RoundPlan plan;
        plan.pass_order = current_order;
        plan.round_seed = actual_seed_used;
        plans.push_back(plan);

        prev_order = current_order;
    }

    return plans;
}

} // namespace orchestration
