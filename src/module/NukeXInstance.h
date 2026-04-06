// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#ifndef __NukeXInstance_h
#define __NukeXInstance_h

#include <pcl/ProcessImplementation.h>
#include <pcl/MetaParameter.h>

namespace pcl
{

class NukeXInstance : public ProcessImplementation
{
public:
   NukeXInstance( const MetaProcess* );
   NukeXInstance( const NukeXInstance& );

   void Assign( const ProcessImplementation& ) override;
   bool IsHistoryUpdater( const View& ) const override;
   UndoFlags UndoMode( const View& ) const override;

   bool CanExecuteOn( const View&, String& whyNot ) const override;
   bool CanExecuteGlobal( String& whyNot ) const override;
   bool ExecuteGlobal() override;
   bool Validate( String& whyNot ) override;

   void* LockParameter( const MetaParameter*, size_type tableRow ) override;
   bool AllocateParameter( size_type sizeOrLength, const MetaParameter*, size_type tableRow ) override;
   size_type ParameterLength( const MetaParameter*, size_type tableRow ) const override;

   // ── Parameter storage ─────────────────────────────────────────

   struct FrameItem {
      String path;
      bool   enabled = true;
   };

   typedef Array<FrameItem> frame_list;

   frame_list  lightFrames;
   frame_list  flatFrames;
   pcl_enum    stretchType     = 0;    // NXStretchType::VeraLux
   pcl_bool    autoStretch     = true;
   pcl_bool    enableGPU       = true;
   String      cacheDirectory  = "/tmp";
};

extern NukeXInstance* TheNukeXInstance;

} // namespace pcl

#endif // __NukeXInstance_h
