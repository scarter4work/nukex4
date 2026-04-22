#include "nukex/learning/train_model.hpp"

namespace nukex::learning {

StretchCoefficients train_one_stretch(sqlite3* /*db*/, const std::string& /*stretch_name*/,
                                      double /*lambda*/, int /*min_rows*/) {
    return {};
}

std::map<std::string, StretchCoefficients>
train_all_stretches(sqlite3* /*db*/, double /*lambda*/) {
    return {};
}

} // namespace nukex::learning
