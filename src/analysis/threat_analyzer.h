#ifndef THREAT_ANALYZER_H
#define THREAT_ANALYZER_H

#include "../../include/interfaces.h"
#include <vector>

namespace analysis {
    std::vector<ThreatReport> analyze_threats(const std::vector<FeatureVector>& features);
}

#endif // THREAT_ANALYZER_H
