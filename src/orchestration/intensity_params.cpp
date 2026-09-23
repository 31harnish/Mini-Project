/**
 * @file intensity_params.cpp
 * @brief Table-driven intensity-to-parameter mapping for each known pass.
 *
 * The table is structured as a map of pass names to arrays of 5 PassParams
 * (one per intensity level 1–5).  To tune behavior, edit the table entries;
 * no call-site changes are needed.
 */

#include "intensity_params.h"
#include <array>
#include <map>
#include <stdexcept>

namespace orchestration {

namespace {

// Each entry: { probability, variant_pool } for intensity levels 1 through 5.
// Monotonic invariant: params[i].probability <= params[i+1].probability
//                      params[i].variant_pool <= params[i+1].variant_pool

// --- Instruction Substitution ---
// At low intensity, substitute ~20% of eligible instructions with 2 variant
// patterns.  At max intensity, substitute all eligible instructions and use
// the full pool of 3 variants for maximum diversity.
constexpr std::array<PassParams, 5> kInstructionSubstitution = {{
    {0.20, 2},   // intensity 1
    {0.40, 2},   // intensity 2
    {0.60, 3},   // intensity 3
    {0.80, 3},   // intensity 4
    {1.00, 3},   // intensity 5
}};

// --- Bogus Control Flow ---
// At low intensity, inject bogus blocks into ~15% of original blocks.
// At max intensity, target ~90% of blocks with 3 predicate families.
constexpr std::array<PassParams, 5> kBogusControlFlow = {{
    {0.15, 2},   // intensity 1
    {0.30, 2},   // intensity 2
    {0.50, 2},   // intensity 3
    {0.70, 3},   // intensity 4
    {0.90, 3},   // intensity 5
}};

// --- Control Flow Flattening ---
// At low intensity, flatten ~10% of functions/blocks (partial mode).
// At max intensity, flatten ~80% with 3 dispatcher variant styles.
constexpr std::array<PassParams, 5> kControlFlowFlattening = {{
    {0.10, 1},   // intensity 1
    {0.25, 1},   // intensity 2
    {0.40, 2},   // intensity 3
    {0.60, 2},   // intensity 4
    {0.80, 3},   // intensity 5
}};

// --- String Obfuscation ---
constexpr std::array<PassParams, 5> kStringObfuscation = {{
    {0.20, 1},   // intensity 1
    {0.40, 1},   // intensity 2
    {0.60, 2},   // intensity 3
    {0.80, 2},   // intensity 4
    {1.00, 2},   // intensity 5
}};

// --- Decoy Idiom ---
constexpr std::array<PassParams, 5> kDecoyIdiom = {{
    {0.10, 1},   // intensity 1
    {0.25, 1},   // intensity 2
    {0.40, 2},   // intensity 3
    {0.60, 2},   // intensity 4
    {0.80, 3},   // intensity 5
}};

// --- Semantic Divergence ---
constexpr std::array<PassParams, 5> kSemanticDivergence = {{
    {0.15, 1},   // intensity 1
    {0.30, 1},   // intensity 2
    {0.50, 2},   // intensity 3
    {0.70, 2},   // intensity 4
    {0.90, 3},   // intensity 5
}};

// --- Adaptive CFG Diversification ---
constexpr std::array<PassParams, 5> kAdaptiveCfgDiversification = {{
    {0.10, 1},   // intensity 1
    {0.25, 1},   // intensity 2
    {0.40, 2},   // intensity 3
    {0.60, 2},   // intensity 4
    {0.80, 3},   // intensity 5
}};

// Lookup table: pass name -> pointer to its params array.
const std::map<std::string, const std::array<PassParams, 5>*> kParamsTable = {
    {"instruction_substitution",      &kInstructionSubstitution},
    {"bogus_control_flow",            &kBogusControlFlow},
    {"control_flow_flattening",       &kControlFlowFlattening},
    {"string_obfuscation",            &kStringObfuscation},
    {"decoy_idiom",                   &kDecoyIdiom},
    {"semantic_divergence",           &kSemanticDivergence},
    {"adaptive_cfg_diversification",  &kAdaptiveCfgDiversification},
};

}  // anonymous namespace

PassParams IntensityToParams(const std::string& pass_name,
                              int intensity_level) {
    if (intensity_level < 1 || intensity_level > 5) {
        throw std::invalid_argument(
            "IntensityToParams: intensity_level must be 1–5, got " +
            std::to_string(intensity_level));
    }

    auto it = kParamsTable.find(pass_name);
    if (it == kParamsTable.end()) {
        throw std::invalid_argument(
            "IntensityToParams: unrecognized pass name '" + pass_name + "'");
    }

    return (*it->second)[static_cast<size_t>(intensity_level - 1)];
}

}  // namespace orchestration
