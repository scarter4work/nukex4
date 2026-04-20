// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#define MODULE_VERSION_MAJOR     4
#define MODULE_VERSION_MINOR     0
#define MODULE_VERSION_REVISION  0
#define MODULE_VERSION_BUILD     6
#define MODULE_VERSION_LANGUAGE  eng

#define MODULE_RELEASE_YEAR      2026
#define MODULE_RELEASE_MONTH     4
#define MODULE_RELEASE_DAY       20

#include "NukeXModule.h"
#include "NukeXProcess.h"
#include "NukeXInterface.h"

namespace pcl
{

NukeXModule::NukeXModule()
{
   TheNukeXModule = this;
}

const char* NukeXModule::Version() const
{
   return PCL_MODULE_VERSION( MODULE_VERSION_MAJOR,
                              MODULE_VERSION_MINOR,
                              MODULE_VERSION_REVISION,
                              MODULE_VERSION_BUILD,
                              MODULE_VERSION_LANGUAGE );
}

IsoString NukeXModule::Name() const
{
   return "NukeX";
}

String NukeXModule::Description() const
{
   return "NukeX v4 — Distribution-Fitted Stacking. "
          "Per-pixel distribution fitting (Student-t, GMM, Contamination, KDE) "
          "with GPU-accelerated weight computation, 10 scientifically validated "
          "stretch operations, and automatic channel/filter detection.";
}

String NukeXModule::Company() const
{
   return "Scott Carter";
}

String NukeXModule::Author() const
{
   return "Scott Carter";
}

String NukeXModule::Copyright() const
{
   return "Copyright (c) 2026 Scott Carter";
}

String NukeXModule::TradeMarks() const
{
   return "NukeX";
}

String NukeXModule::OriginalFileName() const
{
#ifdef __PCL_LINUX
   return "NukeX-pxm.so";
#endif
#ifdef __PCL_MACOSX
   return "NukeX-pxm.dylib";
#endif
#ifdef __PCL_WINDOWS
   return "NukeX-pxm.dll";
#endif
}

void NukeXModule::GetReleaseDate( int& year, int& month, int& day ) const
{
   year  = MODULE_RELEASE_YEAR;
   month = MODULE_RELEASE_MONTH;
   day   = MODULE_RELEASE_DAY;
}

NukeXModule* TheNukeXModule = nullptr;

} // namespace pcl

// ── PCL Module Installation ──────────────────────────────────────
PCL_MODULE_EXPORT int InstallPixInsightModule( int mode )
{
   new pcl::NukeXModule;

   if ( mode == pcl::InstallMode::FullInstall )
   {
      new pcl::NukeXProcess;
      new pcl::NukeXInterface;
   }

   return 0;
}
