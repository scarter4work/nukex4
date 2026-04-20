#include "stretch_factory.hpp"
#include "stretch_auto_selector.hpp"

#include "nukex/stretch/veralux_stretch.hpp"
#include "nukex/stretch/ghs_stretch.hpp"
#include "nukex/stretch/mtf_stretch.hpp"
#include "nukex/stretch/arcsinh_stretch.hpp"
#include "nukex/stretch/log_stretch.hpp"
#include "nukex/stretch/lupton_stretch.hpp"
#include "nukex/stretch/clahe_stretch.hpp"

namespace nukex {

std::unique_ptr<StretchOp> build_primary(PrimaryStretch e,
                                         const FITSMetadata& meta,
                                         std::string& out_log_line) {
    out_log_line.clear();
    if (e == PrimaryStretch::Auto) {
        auto sel = select_auto(classify_filter(meta));
        out_log_line = std::move(sel.log_line);
        return std::move(sel.op);
    }
    switch (e) {
        case PrimaryStretch::VeraLux: return std::make_unique<VeraLuxStretch>();
        case PrimaryStretch::GHS:     return std::make_unique<GHSStretch>();
        case PrimaryStretch::MTF:     return std::make_unique<MTFStretch>();
        case PrimaryStretch::ArcSinh: return std::make_unique<ArcSinhStretch>();
        case PrimaryStretch::Log:     return std::make_unique<LogStretch>();
        case PrimaryStretch::Lupton:  return std::make_unique<LuptonStretch>();
        case PrimaryStretch::CLAHE:   return std::make_unique<CLAHEStretch>();
        case PrimaryStretch::Auto:    break;
    }
    return std::make_unique<VeraLuxStretch>();
}

std::unique_ptr<StretchOp> build_finishing(FinishingStretch /*e*/) {
    return nullptr;
}

} // namespace nukex
