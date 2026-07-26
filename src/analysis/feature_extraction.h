#ifndef FEATURE_EXTRACTION_H
#define FEATURE_EXTRACTION_H

#include "../../include/interfaces.h"

namespace llvm {
    class Module;
}

namespace analysis {
    std::vector<FeatureVector> extract_features(llvm::Module& M);
}

#endif // FEATURE_EXTRACTION_H
