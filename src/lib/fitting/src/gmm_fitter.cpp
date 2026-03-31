#include "nukex/fitting/gmm_fitter.hpp"
namespace nukex {
FitResult GaussianMixtureFitter::fit(const float*, const float*, int, double, double) {
    FitResult r; r.converged = false; return r;
}
} // namespace nukex
