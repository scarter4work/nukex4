#include "nukex/stacker/stacking_engine.hpp"

namespace nukex {

StackingEngine::StackingEngine(const Config& config) : config_(config) {}

StackingEngine::Result StackingEngine::execute(
    const std::vector<std::string>&,
    const std::vector<std::string>&) {
    return {};
}

} // namespace nukex
