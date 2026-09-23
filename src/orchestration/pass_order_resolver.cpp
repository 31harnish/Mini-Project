/**
 * @file pass_order_resolver.cpp
 * @brief Resolves pass execution order using a stable priority sort.
 */

#include "pass_order_resolver.h"
#include <algorithm>

namespace orchestration {

int GetPassPriority(const std::string& pass_name) {
    // Priority 1: instruction-level transforms — operate on individual
    // instructions within existing blocks, must run before any CFG changes.
    if (pass_name == "instruction_substitution" ||
        pass_name == "string_obfuscation" ||
        pass_name == "semantic_divergence") {
        return 1;
    }

    // Priority 2: block-level transforms — add new blocks but don't
    // fundamentally restructure the CFG.
    if (pass_name == "bogus_control_flow" ||
        pass_name == "decoy_idiom") {
        return 2;
    }

    // Priority 3: CFG-restructuring transforms — replace the original
    // block layout with a dispatcher, must run last.
    if (pass_name == "control_flow_flattening" ||
        pass_name == "adaptive_cfg_diversification") {
        return 3;
    }

    // Unknown passes go last as a safety fallback.
    return 99;
}

std::vector<std::string> ResolvePassOrder(
    const std::vector<std::string>& selected_passes) {

    std::vector<std::string> ordered(selected_passes);

    // Stable sort preserves the relative order of passes with the same
    // priority — so if the planner listed two priority-1 passes in a
    // specific order, that order is maintained.
    std::stable_sort(ordered.begin(), ordered.end(),
        [](const std::string& a, const std::string& b) {
            return GetPassPriority(a) < GetPassPriority(b);
        });

    return ordered;
}

}  // namespace orchestration
