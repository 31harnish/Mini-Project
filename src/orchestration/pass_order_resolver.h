#ifndef PASS_ORDER_RESOLVER_H
#define PASS_ORDER_RESOLVER_H

#include <string>
#include <vector>

namespace orchestration {

/**
 * @brief Returns passes in safe execution order.
 *
 * ORDERING CONTRACT:
 *   Priority 1 — instruction-level transforms (instruction_substitution,
 *                string_obfuscation, semantic_divergence):
 *                Must run FIRST because they operate on individual
 *                instructions within existing basic blocks.
 *
 *   Priority 2 — block-level transforms (bogus_control_flow, decoy_idiom):
 *                Run SECOND.  They add new basic blocks but don't
 *                restructure the entire CFG, so instruction-level
 *                transforms can safely precede them.
 *
 *   Priority 3 — CFG-restructuring transforms (control_flow_flattening,
 *                adaptive_cfg_diversification):
 *                Must run LAST.  They fundamentally restructure the CFG
 *                (replacing the original block layout with a dispatcher
 *                switch), so instruction substitution applied after
 *                flattening would operate on dispatcher bookkeeping
 *                rather than user code — producing weaker obfuscation
 *                and risking verifier failures.
 *
 *   Priority 99 — unknown pass names are placed last as a safety fallback.
 *
 * The resolver is a safety net at execution time; Module 3's planner
 * already sorts passes at plan-generation time using the same priorities.
 *
 * @param selected_passes  Pass names in any order.
 * @return The same passes, stable-sorted by priority.
 */
std::vector<std::string> ResolvePassOrder(
    const std::vector<std::string>& selected_passes);

/**
 * @brief Returns the execution priority for a pass name.
 *        Lower number = runs earlier.
 */
int GetPassPriority(const std::string& pass_name);

}  // namespace orchestration

#endif // PASS_ORDER_RESOLVER_H
