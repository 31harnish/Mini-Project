#ifndef EVALUATION_ENGINE_H
#define EVALUATION_ENGINE_H

#include "../../include/interfaces.h"
#include <string>
#include <vector>

namespace eval {

/**
 * @brief Core API for Module 7: Evaluates metrics comparing baseline binary vs obfuscated binary.
 * @param baseline_path Path to the compiled baseline binary.
 * @param obfuscated_path Path to the obfuscated compiled binary.
 * @return EvaluationMetrics populated with size, runtime overhead, and resistance scores.
 */
EvaluationMetrics evaluate_binary(const std::string& baseline_path, const std::string& obfuscated_path);

/**
 * @brief Evaluates binaries while running benchmark execution with custom arguments.
 */
EvaluationMetrics evaluate_binary_with_args(const std::string& baseline_path, 
                                            const std::string& obfuscated_path, 
                                            const std::vector<std::string>& test_args,
                                            int benchmark_runs = 5);

/**
 * @brief Calculates byte-level Shannon Entropy H = -sum(p_i * log2(p_i)) of a file.
 * Returns score between 0.0 (uniform/empty) and 8.0 (maximum entropy/obfuscation).
 */
double calculate_shannon_entropy(const std::string& filepath);

/**
 * @brief Measures total binary file size in bytes using C++ filesystem API.
 */
size_t get_file_size_bytes(const std::string& filepath);

/**
 * @brief Benchmarks process execution time in milliseconds over specified number of runs.
 */
double benchmark_execution_time_ms(const std::string& executable_path, 
                                    const std::vector<std::string>& args, 
                                    int runs = 5);

/**
 * @brief Formats EvaluationMetrics into a human-readable text report.
 */
std::string format_evaluation_report(const EvaluationMetrics& metrics);

} // namespace eval

#endif // EVALUATION_ENGINE_H
