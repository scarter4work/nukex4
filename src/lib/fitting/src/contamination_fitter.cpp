#include "nukex/fitting/contamination_fitter.hpp"
namespace nukex {
FitResult ContaminationFitter::fit(const float*, const float*, int, double, double) {
    FitResult r; r.converged = false; return r;
}
} // namespace nukex
