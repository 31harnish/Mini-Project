#ifndef THREAT_ANALYZER_H
#define THREAT_ANALYZER_H

#include "../../include/interfaces.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace analysis {

class FeatureLoader {
public:
    static std::vector<FeatureVector> loadFromFile(const std::string &filePath);
};

class WeightManager {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, double>> Weights;
    bool Loaded = false;
    void loadDefaults();

public:
    WeightManager();
    void loadWeights(const std::string &filePath);
    double getWeight(const std::string &category, const std::string &key) const;
};

class RiskCalculator {
private:
    const WeightManager &WM;

public:
    RiskCalculator(const WeightManager &wm);
    double calculateScore(const FeatureVector &features) const;
};

class ThreatClassifier {
public:
    static std::string classifyLevel(double score);
    static ProtectionProfile generateProfile(double score);
};

class ReportGenerator {
public:
    static void generateJsonReport(const std::vector<ThreatReport> &reports, const std::string &outputPath);
    static void generateCsvDataset(const std::vector<ThreatReport> &reports, const std::vector<FeatureVector> &features, const std::string &outputPath);
};

// Legacy/Main coordinator API
std::vector<ThreatReport> analyze_threats(const std::vector<FeatureVector>& features);

} // namespace analysis

#endif // THREAT_ANALYZER_H
