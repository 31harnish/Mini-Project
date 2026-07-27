/**
 * @file threat_analyzer.cpp
 * @brief Module 2 — Security & Threat Analyzer
 * 
 * Responsibility: score each function's risk/importance, categorize by
 * sensitivity type, support a configurable threat profile (e.g. strict vs
 * balanced) that adjusts scoring thresholds.
 * 
 * Expected LLVM APIs: None strictly required here, as it operates on FeatureVectors.
 * 
 * Inputs: std::vector<FeatureVector>
 * Outputs: std::vector<ThreatReport>
 * 
 * TODOs for Real Implementation:
 * 1. Define scoring constants for risk weights (e.g., complexity weight, sensitivity multiplier).
 * 2. Implement logic to compute risk_score based on FeatureVector fields.
 * 3. Classify sensitivity_category based on function name or api usage hints.
 * 4. Implement configurable profiles to scale the final risk scores.
 */

#include "threat_analyzer.h"
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <string>

namespace analysis {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

std::vector<ThreatReport> analyze_threats(const std::vector<FeatureVector>& features) {
    std::vector<ThreatReport> reports;
    
    // Read threat profile from environment variable (default: balanced)
    std::string profile = "balanced";
    if (const char* profile_env = std::getenv("THREAT_PROFILE")) {
        profile = to_lower(std::string(profile_env));
    }
    
    // Determine profile multiplier
    double multiplier = 1.0;
    if (profile == "strict") {
        multiplier = 1.3;
    } else if (profile == "lax") {
        multiplier = 0.7;
    }
    
    for (const auto& feature : features) {
        ThreatReport tr;
        tr.function_name = feature.function_name;
        
        // 1. Calculate raw risk score based on complexity and call graph metrics
        double raw_score = 0.0;
        
        // Complexity/size weights
        raw_score += feature.cyclomatic_complexity * 0.02;
        raw_score += feature.basic_block_count * 0.01;
        raw_score += feature.instruction_diversity * 0.005;
        
        // Connectivity weights
        raw_score += feature.call_graph_fan_out * 0.02;
        raw_score += feature.call_graph_fan_in * 0.005;
        
        // Sensitivity weight bonus
        if (feature.is_sensitive) {
            raw_score += 0.40;
        }
        
        // Apply profile multiplier and clamp between 0.0 and 1.0
        tr.risk_score = std::min(1.0, std::max(0.0, raw_score * multiplier));
        
        // 2. Classify sensitivity category
        if (feature.is_sensitive) {
            std::string name_lower = to_lower(feature.function_name);
            if (name_lower.find("login") != std::string::npos || name_lower.find("auth") != std::string::npos) {
                tr.sensitivity_category = "authentication";
            } else if (name_lower.find("encrypt") != std::string::npos || 
                       name_lower.find("decrypt") != std::string::npos ||
                       name_lower.find("crypt") != std::string::npos ||
                       name_lower.find("key") != std::string::npos ||
                       name_lower.find("cipher") != std::string::npos ||
                       name_lower.find("crypto") != std::string::npos) {
                tr.sensitivity_category = "cryptography";
            } else {
                tr.sensitivity_category = "sensitive_generic";
            }
        } else {
            tr.sensitivity_category = "generic";
        }
        
        reports.push_back(tr);
    }
    
    return reports;
}

} // namespace analysis
