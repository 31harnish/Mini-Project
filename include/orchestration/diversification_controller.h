#ifndef DIVERSIFICATION_CONTROLLER_H
#define DIVERSIFICATION_CONTROLLER_H

#include <string>
#include <vector>
#include <cstdint>

namespace orchestration {

struct RoundPlan {
    std::vector<std::string> pass_order;
    uint64_t round_seed;
};

// Generates one RoundPlan per round, 1..max_rounds. Each round's
// pass_order is a seeded permutation of selected_passes.
std::vector<RoundPlan> GenerateRoundPlans(
    const std::vector<std::string>& selected_passes,
    uint64_t base_seed,
    int max_rounds);

} // namespace orchestration

#endif // DIVERSIFICATION_CONTROLLER_H
