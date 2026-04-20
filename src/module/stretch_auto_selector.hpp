#ifndef __NukeX_stretch_auto_selector_h
#define __NukeX_stretch_auto_selector_h

#include "filter_classifier.hpp"
#include "nukex/stretch/stretch_op.hpp"
#include <memory>
#include <string>

namespace nukex {

struct AutoSelection {
    std::unique_ptr<StretchOp> op;
    std::string log_line;
};

AutoSelection select_auto(FilterClass cls);

} // namespace nukex

#endif
