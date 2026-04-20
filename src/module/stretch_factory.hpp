#ifndef __NukeX_stretch_factory_h
#define __NukeX_stretch_factory_h

#include "fits_metadata.hpp"
#include "nukex/stretch/stretch_op.hpp"
#include <memory>
#include <string>

namespace nukex {

// Library-layer enum that mirrors the PCL NXPrimaryStretch enum in
// NukeXParameters.h (Task A6). Values MUST stay synchronized:
// Auto=0, VeraLux=1, GHS=2, MTF=3, ArcSinh=4, Log=5, Lupton=6, CLAHE=7.
enum class PrimaryStretch {
    Auto = 0, VeraLux, GHS, MTF, ArcSinh, Log, Lupton, CLAHE,
};

enum class FinishingStretch {
    None = 0,
};

std::unique_ptr<StretchOp> build_primary(PrimaryStretch e,
                                         const FITSMetadata& meta,
                                         std::string& out_log_line);

std::unique_ptr<StretchOp> build_finishing(FinishingStretch e);

} // namespace nukex

#endif
