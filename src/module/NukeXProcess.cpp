// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXProcess.h"
#include "NukeXParameters.h"
#include <pcl/ErrorHandler.h>
#include "NukeXInstance.h"
#include "NukeXInterface.h"

namespace pcl
{

NukeXProcess* TheNukeXProcess = nullptr;

NukeXProcess::NukeXProcess()
{
   TheNukeXProcess = this;

   // Light frames table + columns
   NXLightFrames* lightTable = new NXLightFrames( this );
   new NXLightFramePath( lightTable );
   new NXLightFrameEnabled( lightTable );

   // Flat frames table + columns
   NXFlatFrames* flatTable = new NXFlatFrames( this );
   new NXFlatFramePath( flatTable );
   new NXFlatFrameEnabled( flatTable );

   // Stretch parameters
   new NXStretchType( this );
   new NXAutoStretch( this );

   // GPU
   new NXEnableGPU( this );

   // Cache
   new NXCacheDirectory( this );
}

IsoString NukeXProcess::Id() const
{
   return "NukeX";
}

IsoString NukeXProcess::Categories() const
{
   return "ImageIntegration";
}

uint32 NukeXProcess::Version() const
{
   return 0x400;  // Version 4.0.0
}

String NukeXProcess::Description() const
{
   return
      "<html>"
      "<p><b>NukeX v4</b> &mdash; Distribution-Fitted Stacking</p>"
      "<p>NukeX integrates subframes using per-pixel distribution fitting "
      "(Student-t, Gaussian Mixture, Contamination, KDE) to determine the "
      "optimal output value for each pixel. Unlike averaging or sigma-clipping, "
      "NukeX fits the actual statistical distribution of pixel values across "
      "all frames and extracts the true signal estimate.</p>"
      "<p><b>Features:</b></p>"
      "<ul>"
      "<li>Per-pixel distribution fitting with AICc model selection</li>"
      "<li>GPU-accelerated weight computation and pixel selection (OpenCL)</li>"
      "<li>10 scientifically validated stretch operations</li>"
      "<li>Automatic filter/channel detection from FITS headers</li>"
      "<li>Flat calibration support</li>"
      "</ul>"
      "</html>";
}

String NukeXProcess::IconImageSVGFile() const
{
   return "@module_icons_dir/NukeX.svg";
}

ProcessInterface* NukeXProcess::DefaultInterface() const
{
   return TheNukeXInterface;
}

ProcessImplementation* NukeXProcess::Create() const
{
   return new NukeXInstance( this );
}

ProcessImplementation* NukeXProcess::Clone( const ProcessImplementation& p ) const
{
   const NukeXInstance* instance = dynamic_cast<const NukeXInstance*>( &p );
   if ( instance == nullptr )
      throw Error( "NukeX: Internal error - cannot clone non-NukeX instance" );
   return new NukeXInstance( *instance );
}

bool NukeXProcess::CanProcessViews() const
{
   return false;  // File-based stacker — no view processing
}

bool NukeXProcess::CanProcessGlobal() const
{
   return true;
}

bool NukeXProcess::IsAssignable() const
{
   return true;
}

bool NukeXProcess::NeedsInitialization() const
{
   return false;  // Instance constructor handles defaults
}

bool NukeXProcess::NeedsValidation() const
{
   return true;
}

bool NukeXProcess::PrefersGlobalExecution() const
{
   return true;  // Always global — file input, not view input
}

} // namespace pcl
