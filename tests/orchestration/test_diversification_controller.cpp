#include "../../include/orchestration/diversification_controller.h"
#include <cassert>
#include <iostream>
#include <algorithm>

using namespace orchestration;

void test_determinism() {
    std::vector<std::string> passes = {"a", "b", "c"};
    auto plans1 = GenerateRoundPlans(passes, 12345, 3);
    auto plans2 = GenerateRoundPlans(passes, 12345, 3);
    
    assert(plans1.size() == 3);
    assert(plans2.size() == 3);
    for (int i = 0; i < 3; ++i) {
        assert(plans1[i].pass_order == plans2[i].pass_order);
        assert(plans1[i].round_seed == plans2[i].round_seed);
    }
    std::cout << "[PASS] determinism\n";
}

void test_different_seeds() {
    std::vector<std::string> passes = {"a", "b", "c", "d", "e"};
    auto plans1 = GenerateRoundPlans(passes, 111, 3);
    auto plans2 = GenerateRoundPlans(passes, 222, 3);
    
    bool diff = false;
    for (int i = 0; i < 3; ++i) {
        if (plans1[i].pass_order != plans2[i].pass_order) {
            diff = true;
            break;
        }
    }
    assert(diff);
    std::cout << "[PASS] different_seeds\n";
}

void test_consecutive_rounds_differ() {
    std::vector<std::string> passes = {"x", "y", "z"};
    auto plans = GenerateRoundPlans(passes, 999, 10);
    for (size_t i = 1; i < plans.size(); ++i) {
        assert(plans[i].pass_order != plans[i-1].pass_order);
    }
    std::cout << "[PASS] consecutive_rounds_differ\n";
}

void test_constraint_substitution_before_flattening() {
    std::vector<std::string> passes = {"control_flow_flattening", "decoy_idiom", "instruction_substitution", "bogus_control_flow"};
    auto plans = GenerateRoundPlans(passes, 42, 50);
    
    for (const auto& plan : plans) {
        auto sub_it = std::find(plan.pass_order.begin(), plan.pass_order.end(), "instruction_substitution");
        auto flat_it = std::find(plan.pass_order.begin(), plan.pass_order.end(), "control_flow_flattening");
        assert(sub_it < flat_it);
    }
    std::cout << "[PASS] constraint_substitution_before_flattening\n";
}

void test_single_pass() {
    std::vector<std::string> passes = {"bogus_control_flow"};
    auto plans = GenerateRoundPlans(passes, 777, 3);
    
    assert(plans.size() == 3);
    for (const auto& plan : plans) {
        assert(plan.pass_order.size() == 1);
        assert(plan.pass_order[0] == "bogus_control_flow");
    }
    std::cout << "[PASS] single_pass\n";
}

void test_constraint_five_element_list() {
    std::vector<std::string> passes = {"control_flow_flattening", "decoy_idiom", "instruction_substitution", "semantic_divergence", "bogus_control_flow"};
    auto plans = GenerateRoundPlans(passes, 555, 5);
    
    for (const auto& plan : plans) {
        auto sub_it = std::find(plan.pass_order.begin(), plan.pass_order.end(), "instruction_substitution");
        auto flat_it = std::find(plan.pass_order.begin(), plan.pass_order.end(), "control_flow_flattening");
        assert(sub_it < flat_it);
        
        // Assert all original pass names are still present exactly once
        assert(plan.pass_order.size() == 5);
        for (const auto& p : passes) {
            assert(std::count(plan.pass_order.begin(), plan.pass_order.end(), p) == 1);
        }
    }
    std::cout << "[PASS] constraint_five_element_list\n";
}

void test_constraint_missing_sub() {
    std::vector<std::string> passes = {"control_flow_flattening", "decoy_idiom", "semantic_divergence"};
    auto plans = GenerateRoundPlans(passes, 666, 3);
    assert(plans.size() == 3);
    for (const auto& plan : plans) {
        assert(plan.pass_order.size() == 3);
        assert(std::count(plan.pass_order.begin(), plan.pass_order.end(), "instruction_substitution") == 0);
        assert(std::count(plan.pass_order.begin(), plan.pass_order.end(), "control_flow_flattening") == 1);
    }
    std::cout << "[PASS] constraint_missing_sub\n";
}

void test_constraint_missing_flat() {
    std::vector<std::string> passes = {"instruction_substitution", "decoy_idiom", "semantic_divergence"};
    auto plans = GenerateRoundPlans(passes, 777, 3);
    assert(plans.size() == 3);
    for (const auto& plan : plans) {
        assert(plan.pass_order.size() == 3);
        assert(std::count(plan.pass_order.begin(), plan.pass_order.end(), "control_flow_flattening") == 0);
        assert(std::count(plan.pass_order.begin(), plan.pass_order.end(), "instruction_substitution") == 1);
    }
    std::cout << "[PASS] constraint_missing_flat\n";
}

int main() {
    std::cout << "Running Diversification Controller tests...\n";
    test_determinism();
    test_different_seeds();
    test_consecutive_rounds_differ();
    test_constraint_substitution_before_flattening();
    test_single_pass();
    test_constraint_five_element_list();
    test_constraint_missing_sub();
    test_constraint_missing_flat();
    std::cout << "All Diversification Controller tests passed!\n";
    return 0;
}
