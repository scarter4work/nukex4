#include "nukex/fitting/student_t_fitter.hpp"
namespace nukex {
FitResult StudentTFitter::fit(const float*, const float*, int, double, double) {
    FitResult r; r.converged = false; return r;
}
} // namespace nukex
