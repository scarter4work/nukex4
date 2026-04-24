// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#ifndef __NukeXProcess_h
#define __NukeXProcess_h

#include <pcl/MetaProcess.h>

namespace pcl
{

class NukeXProcess : public MetaProcess
{
public:
   NukeXProcess();

   IsoString Id() const override;
   IsoString Categories() const override;
   uint32 Version() const override;
   String Description() const override;
   String IconImageSVGFile() const override;
   ProcessInterface* DefaultInterface() const override;
   ProcessImplementation* Create() const override;
   ProcessImplementation* Clone( const ProcessImplementation& ) const override;

   bool CanProcessViews() const override;
   bool CanProcessGlobal() const override;
   bool IsAssignable() const override;
   bool NeedsInitialization() const override;
   bool NeedsValidation() const override;
   bool PrefersGlobalExecution() const override;

   // Phase 8 (Task 17 stub; Task 18 backs with PCL Settings).
   //
   // "Suppressed" means "don't auto-open the rating popup after Execute".
   // The "Rate last run" button on the interface (Task 18) must still work
   // regardless. Const because Task 18 will persist via PCL Settings, not
   // member state.
   bool rating_popup_suppressed() const;
   void set_rating_popup_suppressed(bool v) const;
};

extern NukeXProcess* TheNukeXProcess;

} // namespace pcl

#endif // __NukeXProcess_h
