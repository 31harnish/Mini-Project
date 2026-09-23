#ifndef INTENSITY_PARAMS_H
#define INTENSITY_PARAMS_H

#include <string>

namespace orchestration {

/**
 * @brief Parameters derived from an intensity level for a specific pass.
 *
 * probability  — fraction of eligible sites to transform (0.0–1.0).
 * variant_pool — number of rewrite variants to choose from per site.
 */
struct PassParams {
    double probability  = 0.0;
    int    variant_pool  = 1;
};

/**
 * @brief Table-driven mapping from (pass_name, intensity_level) to PassParams.
 *
 * Monotonic: higher intensity always produces higher probability and
 * at-least-equal variant_pool.  No inline magic numbers — the table
 * is centralized here so intensity behavior can be tuned without
 * touching call logic.
 *
 * @throws std::invalid_argument on unrecognized pass_name or
 *         intensity_level outside [1, 5].
 */
PassParams IntensityToParams(const std::string& pass_name,
                              int intensity_level);

}  // namespace orchestration

#endif // INTENSITY_PARAMS_H
