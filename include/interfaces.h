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
    int cyclomatic_complexity;
    int basic_block_count;
    double cfg_density;
    int instruction_diversity;      // count of distinct instruction opcodes used
    bool is_sensitive;              // true if flagged as login/crypto/auth-like
    int call_graph_fan_in;
    int call_graph_fan_out;
};

/**
 * @brief Represents the threat analysis for a function.
 * Produced by Module 2 (Threat Analyzer), consumed by Module 3 (Protection Planner).
 */
struct ThreatReport {
    std::string function_name;
    double risk_score;              // 0.0 (low) to 1.0 (critical)
    std::string sensitivity_category; // e.g. "authentication", "encryption", "generic"
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
