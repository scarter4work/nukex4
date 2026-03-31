#include "nukex/fitting/model_selector.hpp"
namespace nukex {
ModelSelector::ModelSelector() : config_(Config{}) {}
ModelSelector::ModelSelector(const Config& config) : config_(config) {}
void ModelSelector::select(const float*, const float*, int, SubcubeVoxel&, int) {}
FitResult ModelSelector::select_best(const float*, const float*, int) {
    FitResult r; r.converged = false; return r;
}
} // namespace nukex
