/**
 * @file module4_orchestrator.cpp
 * @brief Module 4 — Existing Obfuscation Passes Orchestrator
 *
 * Orchestrates calls to the obfuscation backend by:
 *   1. Resolving pass execution order (substitution before flattening)
 *   2. Mapping intensity level to pass-specific parameters
 *   3. Invoking each pass sequentially, chaining stage outputs
 *   4. Aborting on the first verification failure
 *   5. Aggregating metrics for downstream consumption
 */

#include "module4_orchestrator.h"
#include "pass_order_resolver.h"
#include "intensity_params.h"
#include <iostream>
#include <stdexcept>

namespace orchestration {

Module4Metrics RunModule4(IObfuscationBackend& backend,
                           const std::string& function_name,
                           const std::vector<std::string>& selected_passes,
                           int intensity_level,
                           uint64_t random_seed,
                           const std::string& ir_in_path,
                           const std::string& ir_out_path) {

    // Step 1: Resolve safe execution order.
    auto ordered = ResolvePassOrder(selected_passes);

    std::string current_in = ir_in_path;
    int total_instructions_added = 0;
    int total_blocks_added       = 0;
    bool fully_deterministic     = true;

    // Step 2–4: For each pass, map intensity -> params, invoke, check result.
    for (size_t i = 0; i < ordered.size(); ++i) {
        const auto& pass = ordered[i];

        // Step 2: Map intensity to backend-specific parameters.
        auto params = IntensityToParams(pass, intensity_level);

        // Determine output path: final pass writes to ir_out_path,
        // intermediate passes write to numbered stage files.
        std::string stage_out;
        if (i + 1 == ordered.size()) {
            stage_out = ir_out_path;
        } else {
            stage_out = ir_out_path + ".stage" + std::to_string(i);
        }

        std::cout << "  [Module4] Invoking pass '" << pass
                  << "' (prob=" << params.probability
                  << ", pool=" << params.variant_pool
                  << ", seed=" << random_seed << ")" << std::endl;

        // Step 3: Invoke the backend.
        auto result = backend.InvokePass(pass, params.probability,
                                          params.variant_pool, random_seed,
                                          current_in, stage_out);

        // Step 4: Abort immediately on verification failure — do NOT
        // continue applying passes to potentially malformed IR.
        if (!result.verified_ok) {
            throw std::runtime_error(
                "Module4: verification failed on pass '" + pass +
                "': " + result.error_message);
        }

        // Flag non-deterministic passes.
        if (!result.seed_respected) {
            std::cout << "  [Module4] WARNING: pass '" << pass
                      << "' does not support seed propagation — "
                      << "output may not be deterministic." << std::endl;
            fully_deterministic = false;
        }

        // Step 5: Aggregate metrics.
        total_instructions_added += result.instructions_added;
        total_blocks_added       += result.blocks_added;

        // Chain: this stage's output becomes the next stage's input.
        current_in = stage_out;
    }

    Module4Metrics metrics{};
    metrics.overhead            = static_cast<double>(total_instructions_added);
    metrics.size_bloat          = static_cast<double>(total_blocks_added);
    metrics.fully_deterministic = fully_deterministic;

    std::cout << "  [Module4] Complete: " << ordered.size() << " passes, "
              << total_instructions_added << " instructions added, "
              << total_blocks_added << " blocks added"
              << (fully_deterministic ? "" : " (NON-DETERMINISTIC)")
              << std::endl;

    return metrics;
}

}  // namespace orchestration
