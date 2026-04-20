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
    switch (e) {
        case PrimaryStretch::Auto: {
            // Pass the full metadata to select_auto so the log line can
            // include the FITS values (FILTER/BAYERPAT/NAXIS3) that drove
            // the classification — useful when a user is debugging an
            // unexpected auto-selection.
            auto sel = select_auto(meta);
            out_log_line = std::move(sel.log_line);
            return std::move(sel.op);
        }
        case PrimaryStretch::VeraLux: return std::make_unique<VeraLuxStretch>();
        case PrimaryStretch::GHS:     return std::make_unique<GHSStretch>();
        case PrimaryStretch::MTF:     return std::make_unique<MTFStretch>();
        case PrimaryStretch::ArcSinh: return std::make_unique<ArcSinhStretch>();
        case PrimaryStretch::Log:     return std::make_unique<LogStretch>();
        case PrimaryStretch::Lupton:  return std::make_unique<LuptonStretch>();
        case PrimaryStretch::CLAHE:   return std::make_unique<CLAHEStretch>();
    }
    [[maybe_unused]] std::unique_ptr<StretchOp> unreachable;
    return unreachable;
}

std::unique_ptr<StretchOp> build_finishing(FinishingStretch /*e*/) {
    return nullptr;
}

} // namespace nukex
