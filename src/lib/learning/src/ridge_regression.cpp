#include "nukex/learning/ridge_regression.hpp"

namespace nukex::learning {

bool fit_ridge(const Eigen::MatrixXd& /*X*/,
               const Eigen::VectorXd& /*y*/,
               double                 /*lambda*/,
               Eigen::VectorXd&       /*coeffs*/) {
    return false;
}

} // namespace nukex::learning
