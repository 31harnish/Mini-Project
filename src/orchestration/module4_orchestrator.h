#ifndef MODULE4_ORCHESTRATOR_H
#define MODULE4_ORCHESTRATOR_H

#include "obfuscation_backend.h"
#include <string>
#include <vector>
#include <cstdint>

namespace orchestration {

/**
 * @brief Aggregated metrics from a Module 4 orchestration run.
 *
 * overhead          — total instructions added across all passes.
 * size_bloat        — total blocks added across all passes.
 * fully_deterministic — false if any pass did not support seed propagation.
 */
struct Module4Metrics {
    double overhead            = 0.0;
    double size_bloat          = 0.0;
    bool   fully_deterministic = true;
};

/**
 * @brief Orchestrate Module 4: resolve pass order, map intensity to
 *        parameters, invoke each pass via the injected backend, and
 *        aggregate metrics.
 *
 * The backend is injected so tests can pass a FakeBackend and
 * production code passes the real one — RunModule4 itself never changes.
 *
 * @throws std::runtime_error if any pass reports a verification failure.
 *         The pipeline aborts immediately; no further passes are applied
 *         to malformed IR.
 * @throws std::invalid_argument if IntensityToParams fails for a pass name.
 */
Module4Metrics RunModule4(IObfuscationBackend& backend,
                           const std::string& function_name,
                           const std::vector<std::string>& selected_passes,
                           int intensity_level,
                           uint64_t random_seed,
                           const std::string& ir_in_path,
                           const std::string& ir_out_path);

}  // namespace orchestration

#endif // MODULE4_ORCHESTRATOR_H
