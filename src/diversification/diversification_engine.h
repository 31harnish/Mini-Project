#ifndef DIVERSIFICATION_ENGINE_H
#define DIVERSIFICATION_ENGINE_H

#include "../../include/interfaces.h"
#include <vector>
#include <string>

namespace diversification {

/**
 * @brief Dispatcher encoding styles for control-flow flattening.
 */
enum class DispatcherStyle {
    SWITCH_CASE,
    INDIRECT_JUMP_TABLE,
    NESTED_IF_ELSE,
    OPAQUE_PREDICATE_CHAINS
};

/**
 * @brief Configuration parameters produced by Module 6 for a diversified pass execution.
 */
struct DiversifiedPassConfig {
    std::string pass_name;
    int seed;
    DispatcherStyle dispatcher_style;
    int bogus_block_density; // percentage (0-100)
    std::string decoy_template_type;
};

/**
 * @brief Comprehensive diversification result for a single function's protection plan.
 */
struct FunctionDiversificationPlan {
    std::string function_name;
    int function_seed;
    std::vector<std::string> randomized_passes;
    std::vector<DiversifiedPassConfig> pass_configs;
};

/**
 * @brief Module 6 Core API: Returns a randomized pass application order for a function.
 * @param plan The protection plan containing selected passes.
 * @param random_seed Seed used to drive randomization.
 * @return Vector of pass names in diversified application order.
 */
std::vector<std::string> get_randomized_pass_order(const ProtectionPlan& plan, int random_seed);

/**
 * @brief Module 6 Helper API: Generates full diversification plan including pass order and pass parameters.
 * @param plan The protection plan containing selected passes and intensity level.
 * @param global_seed Global build seed.
 * @return FunctionDiversificationPlan with randomized pass order and per-pass parameters.
 */
FunctionDiversificationPlan generate_diversified_config(const ProtectionPlan& plan, int global_seed);

/**
 * @brief Helper to generate a unique function-specific seed based on global seed and function name.
 */
int derive_function_seed(const std::string& function_name, int global_seed);

} // namespace diversification

#endif // DIVERSIFICATION_ENGINE_H
