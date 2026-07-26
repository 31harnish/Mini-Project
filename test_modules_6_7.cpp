#include "../src/diversification/diversification_engine.h"
#include "../src/eval/evaluation_engine.h"
#include <iostream>
#include <cassert>

void test_module_6_diversification() {
    std::cout << "--- Testing Module 6: Diversification Engine ---" << std::endl;

    ProtectionPlan plan;
    plan.function_name = "authenticate_user";
    plan.selected_passes = {"bogus_control_flow", "decoy_injection", "control_flow_flattening", "symbol_renaming"};
    plan.intensity_level = 4;
    plan.max_transformation_rounds = 3;

    // Test 1: Function Seed Derivation
    int seed1 = diversification::derive_function_seed(plan.function_name, 12345);
    int seed2 = diversification::derive_function_seed(plan.function_name, 12345);
    int seed3 = diversification::derive_function_seed("other_function", 12345);
    
    assert(seed1 == seed2 && "Same function name and global seed must yield identical derived seed.");
    assert(seed1 != seed3 && "Different function names must yield different derived seeds.");
    std::cout << "[PASS] Seed derivation test successful. Derived Seed: " << seed1 << std::endl;

    // Test 2: Randomized Pass Ordering
    auto shuffled_1 = diversification::get_randomized_pass_order(plan, 12345);
    auto shuffled_2 = diversification::get_randomized_pass_order(plan, 12345);
    auto shuffled_diff = diversification::get_randomized_pass_order(plan, 99999);

    assert(shuffled_1.size() == plan.selected_passes.size() && "Shuffled pass count must match plan pass count.");
    assert(shuffled_1 == shuffled_2 && "Same seed must produce deterministic pass ordering.");
    assert(shuffled_1 != shuffled_diff && "Different seed should produce different pass ordering.");
    
    std::cout << "[PASS] Deterministic & randomized pass order test successful." << std::endl;
    std::cout << "  Original order : bogus_control_flow -> decoy_injection -> control_flow_flattening -> symbol_renaming" << std::endl;
    std::cout << "  Seed 12345 order: ";
    for (const auto& p : shuffled_1) std::cout << p << " ";
    std::cout << std::endl;

    // Test 3: Diversified Config Generation
    auto div_plan = diversification::generate_diversified_config(plan, 12345);
    assert(div_plan.function_name == "authenticate_user");
    assert(div_plan.pass_configs.size() == plan.selected_passes.size());
    std::cout << "[PASS] Full diversification config generation successful." << std::endl;
    for (const auto& cfg : div_plan.pass_configs) {
        std::cout << "  Pass: " << cfg.pass_name 
                  << " | Bogus Density: " << cfg.bogus_block_density << "%"
                  << " | Decoy Template: " << cfg.decoy_template_type << std::endl;
    }
    std::cout << std::endl;
}

void test_module_7_evaluation() {
    std::cout << "--- Testing Module 7: Evaluation Engine ---" << std::endl;

    // Test 1: File size and Shannon entropy on current test binary / source file
    std::string test_file = "CMakeLists.txt";
    size_t file_size = eval::get_file_size_bytes(test_file);
    double entropy = eval::calculate_shannon_entropy(test_file);

    assert(file_size > 0 && "File size of CMakeLists.txt must be > 0");
    assert(entropy > 0.0 && entropy <= 8.0 && "Entropy must be between 0.0 and 8.0");

    std::cout << "[PASS] File size & Shannon entropy test successful." << std::endl;
    std::cout << "  File: " << test_file << " | Size: " << file_size << " bytes | Shannon Entropy: " << entropy << " bits/byte" << std::endl;

    // Test 2: Full Evaluation Metrics Evaluation
    auto metrics = eval::evaluate_binary("out.baseline.bin", "out.obfuscated.bin");
    assert(metrics.binary_id == "out.obfuscated.bin");
    assert(metrics.static_re_resistance_score >= 0.0 && metrics.static_re_resistance_score <= 10.0);
    assert(metrics.llm_reconstruction_accuracy >= 0.0 && metrics.llm_reconstruction_accuracy <= 1.0);

    std::cout << "[PASS] Evaluation metrics generation test successful." << std::endl;
    std::string report = eval::format_evaluation_report(metrics);
    std::cout << report << std::endl;
}

int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << " RUNNING UNIT TESTS FOR MODULE 6 AND MODULE 7 MODULES   " << std::endl;
    std::cout << "========================================================" << std::endl;
    
    test_module_6_diversification();
    test_module_7_evaluation();

    std::cout << "ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}
