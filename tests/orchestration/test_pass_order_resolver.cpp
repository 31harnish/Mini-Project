/**
 * @file test_pass_order_resolver.cpp
 * @brief Unit tests for PassOrderResolver.
 *
 * Verifies:
 *   - All permutations of the 3 core passes resolve to the correct order
 *   - Single-element and empty lists are handled correctly
 *   - Unknown pass names are sorted last
 *   - Passes with equal priority maintain relative (stable) order
 */

#include "../src/orchestration/pass_order_resolver.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

using orchestration::ResolvePassOrder;
using orchestration::GetPassPriority;

// ---- Helpers ----

void AssertOrder(const std::vector<std::string>& result,
                 const std::vector<std::string>& expected,
                 const std::string& test_name) {
    if (result.size() != expected.size()) {
        std::cerr << "[FAIL] " << test_name
                  << ": size mismatch (got " << result.size()
                  << ", expected " << expected.size() << ")\n";
        assert(false);
    }
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i] != expected[i]) {
            std::cerr << "[FAIL] " << test_name
                      << ": at index " << i
                      << " got '" << result[i]
                      << "', expected '" << expected[i] << "'\n";
            assert(false);
        }
    }
    std::cout << "[PASS] " << test_name << "\n";
}

// ---- Tests ----

void test_all_three_already_ordered() {
    auto result = ResolvePassOrder({
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening"
    });
    AssertOrder(result, {
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening"
    }, "all_three_already_ordered");
}

void test_all_three_reversed() {
    auto result = ResolvePassOrder({
        "control_flow_flattening",
        "bogus_control_flow",
        "instruction_substitution"
    });
    AssertOrder(result, {
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening"
    }, "all_three_reversed");
}

void test_all_three_mixed() {
    auto result = ResolvePassOrder({
        "bogus_control_flow",
        "control_flow_flattening",
        "instruction_substitution"
    });
    AssertOrder(result, {
        "instruction_substitution",
        "bogus_control_flow",
        "control_flow_flattening"
    }, "all_three_mixed");
}

void test_substitution_always_before_flattening() {
    // Every permutation of just these two should resolve the same way.
    auto r1 = ResolvePassOrder({
        "instruction_substitution", "control_flow_flattening"
    });
    AssertOrder(r1, {
        "instruction_substitution", "control_flow_flattening"
    }, "sub_before_flat_1");

    auto r2 = ResolvePassOrder({
        "control_flow_flattening", "instruction_substitution"
    });
    AssertOrder(r2, {
        "instruction_substitution", "control_flow_flattening"
    }, "sub_before_flat_2");
}

void test_single_element() {
    auto r1 = ResolvePassOrder({"bogus_control_flow"});
    AssertOrder(r1, {"bogus_control_flow"}, "single_bogus");

    auto r2 = ResolvePassOrder({"instruction_substitution"});
    AssertOrder(r2, {"instruction_substitution"}, "single_substitution");

    auto r3 = ResolvePassOrder({"control_flow_flattening"});
    AssertOrder(r3, {"control_flow_flattening"}, "single_flattening");
}

void test_empty_list() {
    auto result = ResolvePassOrder({});
    AssertOrder(result, {}, "empty_list");
}

void test_unknown_passes_sorted_last() {
    auto result = ResolvePassOrder({
        "some_future_pass",
        "instruction_substitution",
        "another_unknown"
    });
    // substitution (1) should come before unknowns (99).
    assert(result[0] == "instruction_substitution");
    // Both unknowns should be after, in stable (original) order.
    assert(result[1] == "some_future_pass");
    assert(result[2] == "another_unknown");
    std::cout << "[PASS] unknown_passes_sorted_last\n";
}

void test_stable_sort_equal_priority() {
    // Two priority-2 passes should maintain their original relative order.
    auto result = ResolvePassOrder({
        "decoy_idiom",
        "bogus_control_flow"
    });
    assert(result[0] == "decoy_idiom");
    assert(result[1] == "bogus_control_flow");
    std::cout << "[PASS] stable_sort_equal_priority\n";
}

void test_full_planner_pass_set() {
    // Test with the full set of passes the planner can produce.
    auto result = ResolvePassOrder({
        "control_flow_flattening",
        "decoy_idiom",
        "string_obfuscation",
        "bogus_control_flow",
        "instruction_substitution"
    });
    // Priority 1: string_obfuscation, instruction_substitution (stable order from input)
    // Priority 2: decoy_idiom, bogus_control_flow (stable order from input)
    // Priority 3: control_flow_flattening
    assert(GetPassPriority(result[0]) <= GetPassPriority(result[1]));
    assert(GetPassPriority(result[1]) <= GetPassPriority(result[2]));
    assert(GetPassPriority(result[2]) <= GetPassPriority(result[3]));
    assert(GetPassPriority(result[3]) <= GetPassPriority(result[4]));

    // Flattening must be last.
    assert(result[4] == "control_flow_flattening");
    std::cout << "[PASS] full_planner_pass_set\n";
}

void test_priority_values() {
    // Verify the actual priority numbers match the contract.
    assert(GetPassPriority("instruction_substitution") == 1);
    assert(GetPassPriority("string_obfuscation") == 1);
    assert(GetPassPriority("semantic_divergence") == 1);
    assert(GetPassPriority("bogus_control_flow") == 2);
    assert(GetPassPriority("decoy_idiom") == 2);
    assert(GetPassPriority("control_flow_flattening") == 3);
    assert(GetPassPriority("adaptive_cfg_diversification") == 3);
    assert(GetPassPriority("unknown_pass") == 99);
    std::cout << "[PASS] priority_values\n";
}

int main() {
    std::cout << "Running PassOrderResolver tests...\n";
    test_all_three_already_ordered();
    test_all_three_reversed();
    test_all_three_mixed();
    test_substitution_always_before_flattening();
    test_single_element();
    test_empty_list();
    test_unknown_passes_sorted_last();
    test_stable_sort_equal_priority();
    test_full_planner_pass_set();
    test_priority_values();
    std::cout << "All PassOrderResolver tests passed!\n";
    return 0;
}
