#ifndef INTERFACES_H
#define INTERFACES_H

#include <string>
#include <vector>

/**
 * @brief Represents the extracted features of a function.
 * Produced by Module 1 (Feature Extraction), consumed by Module 2 (Threat Analyzer) and Module 3 (Protection Planner).
 */
struct FeatureVector {
    std::string function_name;

    // Structural
    unsigned basic_block_count;
    unsigned instructions;
    unsigned cyclomatic_complexity;
    double cfg_density;

    // Control Flow
    unsigned branch_count;
    unsigned switch_count;
    unsigned loop_count;
    unsigned max_loop_depth;

    // Calls
    unsigned call_count;
    unsigned external_calls;
    bool recursive;

    // Memory
    unsigned loads;
    unsigned stores;
    unsigned allocas;

    // Instruction Mix
    unsigned arithmetic_ops;
    unsigned logic_ops;
    unsigned compare_ops;

    // Security
    bool crypto_api;
    bool auth_api;
    bool file_io;
    bool network_io;

    unsigned secret_strings;
    unsigned integer_constants;

    // Derived
    double instruction_diversity;
    double memory_intensity;
    double string_entropy;

    // Legacy fields for backward compatibility
    bool is_sensitive;
    int call_graph_fan_in;
    int call_graph_fan_out;
};

struct ProtectionProfile {
    std::string obfuscation_intensity; // "None", "Low", "Medium", "High", "Maximum"
    std::string diversification_level; // "None", "Low", "Medium", "High", "Maximum"
    int transformation_rounds;          // 0 to 3
    std::string performance_priority;  // "High", "Medium", "Low"
};

/**
 * @brief Represents the threat analysis for a function.
 * Produced by Module 2 (Threat Analyzer), consumed by Module 3 (Protection Planner).
 */
struct ThreatReport {
    std::string function_name;
    double risk_score;                 // 0.0 to 100.0
    std::string risk_level;            // "Low", "Medium", "High", "Critical"
    ProtectionProfile profile;
    std::string sensitivity_category;  // For legacy compatibility
};

/**
 * @brief Represents the planned protection steps for a function.
 * Produced by Module 3 (Protection Planner), consumed by Module 4 (Existing Passes), Module 5 (Novel Passes), and Module 6 (Diversification Engine).
 */
struct ProtectionPlan {
    std::string function_name;
    std::vector<std::string> selected_passes;   // ordered list of pass names to run
    int intensity_level;             // 1 (light) to 5 (heavy)
    int max_transformation_rounds;   // upper bound for the iterate loop
    double target_llm_reconstruction_accuracy;  // budget threshold (lower = better confused)
};

/**
 * @brief Represents the evaluation results of the protected binary.
 * Produced by Module 7 (Evaluation Engine), consumed by Module 8 (Build Controller) for iterative refinement.
 */
struct EvaluationMetrics {
    std::string binary_id;
    double runtime_overhead_pct;
    double binary_size_growth_pct;
    double cfg_complexity_score;
    double static_re_resistance_score;   // from Ghidra/IDA/angr testing
    double llm_reconstruction_accuracy;  // lower = better resistance
};

#endif // INTERFACES_H
