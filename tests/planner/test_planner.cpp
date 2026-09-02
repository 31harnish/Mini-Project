#include "../../src/planner/protection_planner.h"
#include "../../src/planner/planner_utils.h"
#include <iostream>
#include <cassert>

using namespace planner;

void test_low_risk() {
    FeatureVector fv = {"low_risk_func", 10, 20, 0.5, 15, false, 2, 2};
    ThreatReport tr = {"low_risk_func", 0.1, "generic"};
    
    auto plans = generate_plan({tr}, {fv});
    assert(plans.size() == 1);
    assert(plans[0].intensity_level == 1);
    assert(plans[0].max_transformation_rounds == 1);
    assert(plans[0].selected_passes.size() == 1);
    assert(plans[0].selected_passes[0] == "instruction_substitution");
    std::cout << "[PASS] test_low_risk\n";
}

void test_high_risk() {
    FeatureVector fv = {"high_risk_func", 20, 50, 0.6, 20, false, 5, 5};
    ThreatReport tr = {"high_risk_func", 0.8, "generic"};
    
    auto plans = generate_plan({tr}, {fv});
    assert(plans.size() == 1);
    assert(plans[0].intensity_level == 4);
    assert(plans[0].max_transformation_rounds == 2);
    // order: decoy_idiom, bogus_control_flow, control_flow_flattening
    assert(plans[0].selected_passes.size() == 3);
    assert(plans[0].selected_passes[0] == "decoy_idiom" || plans[0].selected_passes[0] == "bogus_control_flow");
    std::cout << "[PASS] test_high_risk\n";
}

void test_auth() {
    FeatureVector fv = {"verify_login", 15, 30, 0.5, 20, true, 2, 3};
    ThreatReport tr = {"verify_login", 0.95, "authentication"};
    
    auto plans = generate_plan({tr}, {fv});
    assert(plans.size() == 1);
    assert(plans[0].intensity_level == 5);
    assert(plans[0].max_transformation_rounds == 3);
    // "authentication" forces string_obfuscation and decoy_idiom
    bool has_string = false;
    for(auto p : plans[0].selected_passes) if(p == "string_obfuscation") has_string = true;
    assert(has_string);
    std::cout << "[PASS] test_auth\n";
}

void test_overhead_budget() {
    // Large, complex function but NOT sensitive
    FeatureVector fv = {"bloated_func", 60, 600, 0.9, 100, false, 10, 10};
    ThreatReport tr = {"bloated_func", 0.95, "generic"};
    
    auto plans = generate_plan({tr}, {fv});
    assert(plans.size() == 1);
    // Should be capped at 4
    assert(plans[0].intensity_level == 4);
    std::cout << "[PASS] test_overhead_budget\n";
}

int main() {
    std::cout << "Running planner tests...\n";
    test_low_risk();
    test_high_risk();
    test_auth();
    test_overhead_budget();
    std::cout << "All tests passed!\n";
    return 0;
}
