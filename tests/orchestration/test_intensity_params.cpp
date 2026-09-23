/**
 * @file test_intensity_params.cpp
 * @brief Unit tests for IntensityToParams.
 *
 * Verifies:
 *   - All 5 intensity levels for each known pass produce valid params
 *   - Monotonic scaling: higher intensity -> higher probability, >= variant_pool
 *   - Unknown pass name throws std::invalid_argument
 *   - Out-of-range intensity (0, 6) throws std::invalid_argument
 */

#include "../src/orchestration/intensity_params.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

using orchestration::IntensityToParams;
using orchestration::PassParams;

// ---- Helpers ----

void AssertMonotonic(const std::string& pass_name) {
    PassParams prev = IntensityToParams(pass_name, 1);

    for (int level = 2; level <= 5; ++level) {
        PassParams curr = IntensityToParams(pass_name, level);

        if (curr.probability < prev.probability) {
            std::cerr << "[FAIL] " << pass_name
                      << ": probability not monotonic at level " << level
                      << " (got " << curr.probability
                      << " < " << prev.probability << ")\n";
            assert(false);
        }
        if (curr.variant_pool < prev.variant_pool) {
            std::cerr << "[FAIL] " << pass_name
                      << ": variant_pool not monotonic at level " << level
                      << " (got " << curr.variant_pool
                      << " < " << prev.variant_pool << ")\n";
            assert(false);
        }
        prev = curr;
    }
    std::cout << "[PASS] monotonic_" << pass_name << "\n";
}

// ---- Tests ----

void test_monotonic_all_passes() {
    std::vector<std::string> passes = {
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening",
        "string_obfuscation",
        "decoy_idiom",
        "semantic_divergence",
        "adaptive_cfg_diversification",
    };
    for (const auto& pass : passes) {
        AssertMonotonic(pass);
    }
}

void test_valid_ranges() {
    // Probabilities should be in [0.0, 1.0], variant_pool >= 1.
    std::vector<std::string> passes = {
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening",
    };
    for (const auto& pass : passes) {
        for (int level = 1; level <= 5; ++level) {
            auto p = IntensityToParams(pass, level);
            assert(p.probability >= 0.0 && p.probability <= 1.0);
            assert(p.variant_pool >= 1);
        }
    }
    std::cout << "[PASS] valid_ranges\n";
}

void test_specific_values() {
    // Spot-check a few known table entries.
    auto p1 = IntensityToParams("instruction_substitution", 1);
    assert(p1.probability >= 0.19 && p1.probability <= 0.21);  // ~0.20
    assert(p1.variant_pool == 2);

    auto p5 = IntensityToParams("instruction_substitution", 5);
    assert(p5.probability >= 0.99 && p5.probability <= 1.01);  // ~1.00
    assert(p5.variant_pool == 3);

    auto bcf3 = IntensityToParams("bogus_control_flow", 3);
    assert(bcf3.probability >= 0.49 && bcf3.probability <= 0.51);  // ~0.50
    assert(bcf3.variant_pool == 2);

    std::cout << "[PASS] specific_values\n";
}

void test_unknown_pass_throws() {
    bool threw = false;
    try {
        IntensityToParams("nonexistent_pass", 3);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
    std::cout << "[PASS] unknown_pass_throws\n";
}

void test_intensity_out_of_range_throws() {
    // intensity 0
    bool threw_low = false;
    try {
        IntensityToParams("instruction_substitution", 0);
    } catch (const std::invalid_argument&) {
        threw_low = true;
    }
    assert(threw_low);

    // intensity 6
    bool threw_high = false;
    try {
        IntensityToParams("instruction_substitution", 6);
    } catch (const std::invalid_argument&) {
        threw_high = true;
    }
    assert(threw_high);

    // negative intensity
    bool threw_neg = false;
    try {
        IntensityToParams("instruction_substitution", -1);
    } catch (const std::invalid_argument&) {
        threw_neg = true;
    }
    assert(threw_neg);

    std::cout << "[PASS] intensity_out_of_range_throws\n";
}

int main() {
    std::cout << "Running IntensityToParams tests...\n";
    test_monotonic_all_passes();
    test_valid_ranges();
    test_specific_values();
    test_unknown_pass_throws();
    test_intensity_out_of_range_throws();
    std::cout << "All IntensityToParams tests passed!\n";
    return 0;
}
