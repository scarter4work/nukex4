#include "nukex/fitting/kde_fitter.hpp"
namespace nukex {
FitResult KDEFitter::fit(const float*, const float*, int, double, double) {
    FitResult r; r.converged = false; return r;
}
double KDEFitter::evaluate_kde(double, const float*, int, double) { return 0.0; }
double KDEFitter::find_mode(const float*, int, double, double, double) { return 0.0; }
double KDEFitter::isj_bandwidth(const float*, int) { return 1.0; }
} // namespace nukex
