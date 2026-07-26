#ifndef DIVERSIFICATION_ENGINE_H
#define DIVERSIFICATION_ENGINE_H

#include "../../include/interfaces.h"
#include <vector>
#include <string>

namespace diversification {
    std::vector<std::string> get_randomized_pass_order(const ProtectionPlan& plan, int random_seed);
}

#endif // DIVERSIFICATION_ENGINE_H
