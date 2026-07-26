#ifndef EVALUATION_ENGINE_H
#define EVALUATION_ENGINE_H

#include "../../include/interfaces.h"
#include <string>

namespace eval {
    EvaluationMetrics evaluate_binary(const std::string& baseline_path, const std::string& obfuscated_path);
}

#endif // EVALUATION_ENGINE_H
