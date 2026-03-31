#include "nukex/combine/spatial_context.hpp"
#include "nukex/core/cube.hpp"
namespace nukex {
float SpatialContext::sobel_gradient(const Image&, int, int) { return 0.0f; }
void SpatialContext::compute(const Image&, Cube&) const {}
} // namespace nukex
