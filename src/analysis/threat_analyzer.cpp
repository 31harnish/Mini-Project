#include "threat_analyzer.h"
#include <llvm/Support/JSON.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace analysis {

// -------------------------------------------------------------------
// FeatureLoader Implementation
// -------------------------------------------------------------------
std::vector<FeatureVector> FeatureLoader::loadFromFile(const std::string &filePath) {
    std::vector<FeatureVector> features;
    
    auto bufferOrErr = llvm::MemoryBuffer::getFile(filePath);
    if (!bufferOrErr) {
        llvm::errs() << "Error: Could not read features file " << filePath << "\n";
        return features;
    }
    
    auto parseResult = llvm::json::parse(bufferOrErr.get()->getBuffer());
    if (!parseResult) {
        llvm::errs() << "Error: Failed to parse features JSON.\n";
        return features;
    }
    
    auto *arr = parseResult->getAsArray();
    if (!arr) return features;
    
    for (const auto &val : *arr) {
        auto *obj = val.getAsObject();
        if (!obj) continue;
        
        FeatureVector fv;
        fv.function_name = obj->getString("function").value_or("").str();
        fv.basic_block_count = obj->getInteger("basic_blocks").value_or(0);
        fv.instructions = obj->getInteger("instructions").value_or(0);
        fv.cyclomatic_complexity = obj->getInteger("cyclomatic").value_or(0);
        fv.cfg_density = obj->getNumber("cfg_density").value_or(0.0);
        fv.branch_count = obj->getInteger("branches").value_or(0);
        fv.switch_count = obj->getInteger("switches").value_or(0);
        fv.loop_count = obj->getInteger("loops").value_or(0);
        fv.max_loop_depth = obj->getInteger("max_loop_depth").value_or(0);
        fv.call_count = obj->getInteger("calls").value_or(0);
        fv.external_calls = obj->getInteger("external_calls").value_or(0);
        fv.recursive = obj->getBoolean("recursive").value_or(false);
        fv.loads = obj->getInteger("loads").value_or(0);
        fv.stores = obj->getInteger("stores").value_or(0);
        fv.allocas = obj->getInteger("allocas").value_or(0);
        fv.arithmetic_ops = obj->getInteger("arithmetic_ops").value_or(0);
        fv.logic_ops = obj->getInteger("logic_ops").value_or(0);
        fv.compare_ops = obj->getInteger("compare_ops").value_or(0);
        fv.crypto_api = obj->getBoolean("crypto").value_or(false);
        fv.auth_api = obj->getBoolean("authentication").value_or(false);
        fv.file_io = obj->getBoolean("fileio").value_or(false);
        fv.network_io = obj->getBoolean("networkio").value_or(false);
        fv.secret_strings = obj->getInteger("secret_strings").value_or(0);
        fv.integer_constants = obj->getInteger("integer_constants").value_or(0);
        fv.instruction_diversity = obj->getNumber("instruction_diversity").value_or(0.0);
        fv.memory_intensity = obj->getNumber("memory_intensity").value_or(0.0);
        
        // Backward compatibility mappings
        fv.is_sensitive = obj->getBoolean("is_sensitive").value_or(fv.crypto_api || fv.auth_api);
        fv.call_graph_fan_in = obj->getInteger("fan_in").value_or(0);
        fv.call_graph_fan_out = obj->getInteger("fan_out").value_or(0);
        
        features.push_back(fv);
    }
    
    return features;
}

// -------------------------------------------------------------------
// WeightManager Implementation
// -------------------------------------------------------------------
WeightManager::WeightManager() {
    loadDefaults();
    
    std::string configPath = "ThreatWeights.json";
    // Check local or workspace path
    if (!llvm::sys::fs::exists(configPath)) {
        configPath = "/Users/guru/Downloads/Mini-Project/ThreatWeights.json";
    }
    
    if (llvm::sys::fs::exists(configPath)) {
        loadWeights(configPath);
    }
}

void WeightManager::loadDefaults() {
    Weights["structural"]["cyclomatic_complexity"] = 2.0;
    Weights["structural"]["cfg_density"] = 15.0;
    Weights["structural"]["basic_blocks"] = 0.5;

    Weights["behavioral"]["loops"] = 5.0;
    Weights["behavioral"]["recursion"] = 15.0;

    Weights["security"]["crypto"] = 35.0;
    Weights["security"]["auth"] = 30.0;
    Weights["security"]["secrets"] = 20.0;

    Weights["resource"]["file_io"] = 10.0;
    Weights["resource"]["network_io"] = 15.0;
    Weights["resource"]["memory_intensity"] = 10.0;
    
    Loaded = true;
}

void WeightManager::loadWeights(const std::string &filePath) {
    auto bufferOrErr = llvm::MemoryBuffer::getFile(filePath);
    if (!bufferOrErr) return;
    
    auto parseResult = llvm::json::parse(bufferOrErr.get()->getBuffer());
    if (!parseResult) return;
    
    auto *obj = parseResult->getAsObject();
    if (!obj) return;
    
    auto extractSubWeights = [&](const std::string &category) {
        if (auto *subObj = obj->getObject(category)) {
            for (auto &kv : *subObj) {
                if (auto val = kv.second.getAsNumber()) {
                    Weights[category][kv.first.str()] = *val;
                }
            }
        }
    };
    
    extractSubWeights("structural");
    extractSubWeights("behavioral");
    extractSubWeights("security");
    extractSubWeights("resource");
}

double WeightManager::getWeight(const std::string &category, const std::string &key) const {
    auto catIt = Weights.find(category);
    if (catIt != Weights.end()) {
        auto keyIt = catIt->second.find(key);
        if (keyIt != catIt->second.end()) {
            return keyIt->second;
        }
    }
    return 0.0;
}

// -------------------------------------------------------------------
// RiskCalculator Implementation
// -------------------------------------------------------------------
RiskCalculator::RiskCalculator(const WeightManager &wm) : WM(wm) {}

double RiskCalculator::calculateScore(const FeatureVector &features) const {
    // 1. Structural Risk
    double structuralRisk = 
        features.cyclomatic_complexity * WM.getWeight("structural", "cyclomatic_complexity") +
        features.cfg_density * WM.getWeight("structural", "cfg_density") +
        features.basic_block_count * WM.getWeight("structural", "basic_blocks");

    // 2. Behavioral Risk
    double behavioralRisk = 
        features.loop_count * WM.getWeight("behavioral", "loops") +
        (features.recursive ? 1.0 : 0.0) * WM.getWeight("behavioral", "recursion");

    // 3. Security Risk
    double securityRisk = 
        (features.crypto_api ? 1.0 : 0.0) * WM.getWeight("security", "crypto") +
        (features.auth_api ? 1.0 : 0.0) * WM.getWeight("security", "auth") +
        features.secret_strings * WM.getWeight("security", "secrets");

    // 4. Resource Risk
    double resourceRisk = 
        (features.file_io ? 1.0 : 0.0) * WM.getWeight("resource", "file_io") +
        (features.network_io ? 1.0 : 0.0) * WM.getWeight("resource", "network_io") +
        features.memory_intensity * WM.getWeight("resource", "memory_intensity");

    double totalScore = structuralRisk + behavioralRisk + securityRisk + resourceRisk;
    
    // Clamp between 0.0 and 100.0
    return std::min(100.0, std::max(0.0, totalScore));
}

// -------------------------------------------------------------------
// ThreatClassifier Implementation
// -------------------------------------------------------------------
std::string ThreatClassifier::classifyLevel(double score) {
    if (score <= 30.0) return "Low";
    if (score <= 60.0) return "Medium";
    if (score <= 80.0) return "High";
    return "Critical";
}

ProtectionProfile ThreatClassifier::generateProfile(double score) {
    ProtectionProfile profile;
    if (score <= 30.0) {
        profile.obfuscation_intensity = "Low";
        profile.diversification_level = "Low";
        profile.transformation_rounds = 1;
        profile.performance_priority = "High";
    } else if (score <= 60.0) {
        profile.obfuscation_intensity = "Medium";
        profile.diversification_level = "Medium";
        profile.transformation_rounds = 1;
        profile.performance_priority = "Medium";
    } else if (score <= 80.0) {
        profile.obfuscation_intensity = "High";
        profile.diversification_level = "High";
        profile.transformation_rounds = 2;
        profile.performance_priority = "Low";
    } else {
        profile.obfuscation_intensity = "Maximum";
        profile.diversification_level = "Maximum";
        profile.transformation_rounds = 3;
        profile.performance_priority = "Low";
    }
    return profile;
}

// -------------------------------------------------------------------
// ReportGenerator Implementation
// -------------------------------------------------------------------
void ReportGenerator::generateJsonReport(const std::vector<ThreatReport> &reports, const std::string &outputPath) {
    llvm::json::Array jsonList;
    
    for (const auto &tr : reports) {
        llvm::json::Object jsonReport;
        jsonReport["Function"] = tr.function_name;
        jsonReport["RiskScore"] = tr.risk_score;
        jsonReport["RiskLevel"] = tr.risk_level;
        
        llvm::json::Object jsonProfile;
        jsonProfile["ObfuscationIntensity"] = tr.profile.obfuscation_intensity;
        jsonProfile["DiversificationLevel"] = tr.profile.diversification_level;
        jsonProfile["TransformationRounds"] = tr.profile.transformation_rounds;
        jsonProfile["PerformancePriority"] = tr.profile.performance_priority;
        
        jsonReport["ProtectionProfile"] = std::move(jsonProfile);
        jsonList.push_back(std::move(jsonReport));
    }
    
    std::error_code ec;
    llvm::raw_fd_ostream os(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Error: Could not open output file for threat report: " << ec.message() << "\n";
        return;
    }
    
    os << llvm::formatv("{0:2}", llvm::json::Value(std::move(jsonList)));
}

void ReportGenerator::generateCsvDataset(const std::vector<ThreatReport> &reports, const std::vector<FeatureVector> &features, const std::string &outputPath) {
    std::error_code ec;
    llvm::raw_fd_ostream os(outputPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "Error: Could not open output file for CSV report: " << ec.message() << "\n";
        return;
    }
    
    os << "Function Name,Block_Count,Complexity,Inst_Count,Loop_Depth,Has_Crypto,String_Entropy,Target Risk Score\n";
    
    for (size_t i = 0; i < reports.size() && i < features.size(); ++i) {
        const auto &tr = reports[i];
        const auto &fv = features[i];
        
        os << "\"" << tr.function_name << "\","
           << fv.basic_block_count << ","
           << fv.cyclomatic_complexity << ","
           << fv.instructions << ","
           << fv.max_loop_depth << ","
           << (fv.crypto_api ? 1 : 0) << ","
           << fv.string_entropy << ","
           << (tr.risk_score / 100.0) << "\n";
    }
}

// -------------------------------------------------------------------
// Main Coordinator Function
// -------------------------------------------------------------------
std::vector<ThreatReport> analyze_threats(const std::vector<FeatureVector>& features) {
    std::vector<ThreatReport> reports;
    
    WeightManager wm;
    RiskCalculator calculator(wm);
    
    for (const auto &fv : features) {
        ThreatReport tr;
        tr.function_name = fv.function_name;
        tr.risk_score = calculator.calculateScore(fv);
        tr.risk_level = ThreatClassifier::classifyLevel(tr.risk_score);
        tr.profile = ThreatClassifier::generateProfile(tr.risk_score);
        
        // Populate legacy sensitivity categories for backwards compatibility
        if (fv.is_sensitive) {
            tr.sensitivity_category = fv.crypto_api ? "cryptography" : "authentication";
        } else {
            tr.sensitivity_category = "generic";
        }
        
        reports.push_back(tr);
    }
    
    // Save report to risk.json and training dataset to features_dataset.csv
    ReportGenerator::generateJsonReport(reports, "risk.json");
    ReportGenerator::generateCsvDataset(reports, features, "features_dataset.csv");
    
    return reports;
}

} // namespace analysis
