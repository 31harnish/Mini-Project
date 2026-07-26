#ifndef PROTECTION_PLANNER_H
#define PROTECTION_PLANNER_H

#include "../../include/interfaces.h"
#include <vector>

namespace planner {
    std::vector<ProtectionPlan> generate_plan(
        const std::vector<ThreatReport>& reports,
        const std::vector<FeatureVector>& features);
}

#endif // PROTECTION_PLANNER_H
