// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXInstance.h"
#include "NukeXParameters.h"
#include "NukeXProcess.h"

#include "NukeXProgress.h"
#include <pcl/ImageWindow.h>
#include <pcl/View.h>

// NukeX pipeline headers
#include "nukex/stacker/stacking_engine.hpp"
#include "nukex/stretch/stretch_pipeline.hpp"
#include "fits_metadata.hpp"
#include "stretch_factory.hpp"

namespace pcl
{

NukeXInstance::NukeXInstance( const MetaProcess* m )
   : ProcessImplementation( m )
{
}

NukeXInstance::NukeXInstance( const NukeXInstance& x )
   : ProcessImplementation( x )
{
   Assign( x );
}

void NukeXInstance::Assign( const ProcessImplementation& p )
{
   const NukeXInstance* x = dynamic_cast<const NukeXInstance*>( &p );
   if ( x != nullptr )
   {
      lightFrames    = x->lightFrames;
      flatFrames     = x->flatFrames;
      primaryStretch   = x->primaryStretch;
      finishingStretch = x->finishingStretch;
      enableGPU      = x->enableGPU;
      cacheDirectory = x->cacheDirectory;
   }
}

bool NukeXInstance::IsHistoryUpdater( const View& ) const
{
   return false;  // Global process — doesn't modify existing views
}

UndoFlags NukeXInstance::UndoMode( const View& ) const
{
   return UndoFlag::DefaultMode;
}

bool NukeXInstance::CanExecuteOn( const View&, String& whyNot ) const
{
   whyNot = "NukeX is a global process. It cannot be executed on views.";
   return false;
}

bool NukeXInstance::Validate( String& whyNot )
{
   if ( lightFrames.IsEmpty() )
   {
      whyNot = "No light frames specified.";
      return false;
   }
   // Check that at least one frame is enabled
   bool any_enabled = false;
   for ( const auto& f : lightFrames )
      if ( f.enabled ) { any_enabled = true; break; }
   if ( !any_enabled )
   {
      whyNot = "No light frames are enabled.";
      return false;
   }
   return true;
}

bool NukeXInstance::CanExecuteGlobal( String& whyNot ) const
{
   if ( lightFrames.IsEmpty() )
   {
      whyNot = "No light frames specified.";
      return false;
   }
   return true;
}

bool NukeXInstance::ExecuteGlobal()
{
   NukeXProgress progress;
   progress.message( "NukeX v4 \xe2\x80\x94 Distribution-Fitted Stacking" );
   progress.message( String().Format( "Processing %zu light frame(s)", lightFrames.Length() ).ToUTF8().c_str() );

   // Collect enabled file paths
   std::vector<std::string> light_paths;
   for ( const auto& f : lightFrames )
   {
      if ( f.enabled )
         light_paths.push_back( f.path.ToUTF8().c_str() );
   }

   std::vector<std::string> flat_paths;
   for ( const auto& f : flatFrames )
   {
      if ( f.enabled )
         flat_paths.push_back( f.path.ToUTF8().c_str() );
   }

   if ( light_paths.empty() )
   {
      progress.message( "** No enabled light frames. Aborting." );
      return false;
   }

   // Configure stacking engine
   nukex::StackingEngine::Config config;
   config.cache_dir = cacheDirectory.ToUTF8().c_str();
   config.gpu_config.force_cpu_fallback = !enableGPU;

   // Execute pipeline with progress reporting
   nukex::StackingEngine engine( config );
   auto result = engine.execute( light_paths, flat_paths, &progress );

   // Check cancellation
   if ( progress.is_cancelled() )
   {
      progress.message( "** Cancelled by user." );
      return false;
   }

   if ( result.n_frames_processed == 0 )
   {
      progress.message( "** No frames were processed. Check input files." );
      return false;
   }

   // Publish result counts to PJSR-readable output parameters before logging.
   nFramesProcessed        = result.n_frames_processed;
   nFramesFailedAlignment  = result.n_frames_failed_alignment;

   progress.message( String().Format(
      "Stacking complete: %d frame(s) processed, %d failed alignment",
      result.n_frames_processed, result.n_frames_failed_alignment ).ToUTF8().c_str() );

   // Create output ImageWindow with the stacked result
   if ( !result.stacked.empty() )
   {
      int w = result.stacked.width();
      int h = result.stacked.height();
      int nc = result.stacked.n_channels();

      ImageWindow window( w, h, nc,
                          32,    // bits per sample (float32)
                          true,  // float sample
                          nc >= 3, // color if 3+ channels
                          true,  // initialProcessing
                          "NukeX_stacked" );

      View view = window.MainView();
      ImageVariant v = view.Image();

      if ( v.IsFloatSample() && v.BitsPerSample() == 32 )
      {
         pcl::Image& img = static_cast<pcl::Image&>( *v );
         for ( int ch = 0; ch < nc; ch++ )
         {
            const float* src = result.stacked.channel_data( ch );
            float* dst = img.PixelData( ch );
            ::memcpy( dst, src, w * h * sizeof( float ) );
         }
      }

      window.Show();
      progress.message( "Stacked image opened." );
   }

   // ── Stretch pipeline (Phase 7 wiring) ─────────────────────────
   if ( !result.stacked.empty() && !light_paths.empty() )
   {
      nukex::FITSMetadata meta = nukex::read_fits_metadata( light_paths.front() );
      std::string auto_log;
      auto primary_op   = nukex::build_primary(
          static_cast<nukex::PrimaryStretch>( primaryStretch ), meta, auto_log );
      auto finishing_op = nukex::build_finishing(
          static_cast<nukex::FinishingStretch>( finishingStretch ) );

      if ( !auto_log.empty() )
         progress.message( auto_log.c_str() );

      // Deep copy — stretch is in-place; must not mutate result.stacked.
      // Safe: nukex::Image stores pixels in std::vector<float>, so operator=
      // performs a full element-wise deep copy (no shared buffer).
      nukex::Image stretched = result.stacked;

      nukex::StretchPipeline pipeline;
      if ( primary_op )
      {
         primary_op->enabled  = true;
         primary_op->position = 0;
         pipeline.ops.push_back( std::move( primary_op ) );
      }
      if ( finishing_op )
      {
         finishing_op->enabled  = true;
         finishing_op->position = 1;
         pipeline.ops.push_back( std::move( finishing_op ) );
      }
      pipeline.execute( stretched );

      int sw  = stretched.width();
      int sh  = stretched.height();
      int snc = stretched.n_channels();

      ImageWindow sw_win( sw, sh, snc, 32, true, snc >= 3, true, "NukeX_stretched" );
      View sv = sw_win.MainView();
      ImageVariant svi = sv.Image();
      if ( svi.IsFloatSample() && svi.BitsPerSample() == 32 )
      {
         pcl::Image& si = static_cast<pcl::Image&>( *svi );
         for ( int ch = 0; ch < snc; ch++ )
         {
            const float* src = stretched.channel_data( ch );
            float* dst = si.PixelData( ch );
            ::memcpy( dst, src, sw * sh * sizeof( float ) );
         }
      }
      sw_win.Show();
      progress.message( "Stretched image opened." );
   }

   // Create noise map window
   if ( !result.noise_map.empty() )
   {
      int w = result.noise_map.width();
      int h = result.noise_map.height();
      int nc = result.noise_map.n_channels();

      ImageWindow nw( w, h, nc, 32, true, nc >= 3, true, "NukeX_noise" );
      View nv = nw.MainView();
      ImageVariant nvi = nv.Image();

      if ( nvi.IsFloatSample() && nvi.BitsPerSample() == 32 )
      {
         pcl::Image& ni = static_cast<pcl::Image&>( *nvi );
         for ( int ch = 0; ch < nc; ch++ )
         {
            const float* src = result.noise_map.channel_data( ch );
            float* dst = ni.PixelData( ch );
            ::memcpy( dst, src, w * h * sizeof( float ) );
         }
      }

      nw.Show();
      progress.message( "Noise map opened." );
   }

   progress.message( "NukeX v4 done." );
   return true;
}

// ── Parameter serialization ──────────────────────────────────────

void* NukeXInstance::LockParameter( const MetaParameter* p, size_type tableRow )
{
   if ( p == TheNXLightFramePathParameter )    return lightFrames[tableRow].path.Begin();
   if ( p == TheNXLightFrameEnabledParameter ) return &lightFrames[tableRow].enabled;
   if ( p == TheNXFlatFramePathParameter )     return flatFrames[tableRow].path.Begin();
   if ( p == TheNXFlatFrameEnabledParameter )  return &flatFrames[tableRow].enabled;
   if ( p == TheNXPrimaryStretchParameter )    return &primaryStretch;
   if ( p == TheNXFinishingStretchParameter )  return &finishingStretch;
   if ( p == TheNXEnableGPUParameter )         return &enableGPU;
   if ( p == TheNXCacheDirectoryParameter )    return cacheDirectory.Begin();
   if ( p == TheNXNFramesProcessedParameter )       return &nFramesProcessed;
   if ( p == TheNXNFramesFailedAlignmentParameter ) return &nFramesFailedAlignment;
   return nullptr;
}

bool NukeXInstance::AllocateParameter( size_type sizeOrLength, const MetaParameter* p, size_type tableRow )
{
   if ( p == TheNXLightFramesParameter )
   {
      lightFrames.Clear();
      if ( sizeOrLength > 0 )
         lightFrames.Add( FrameItem(), sizeOrLength );
   }
   else if ( p == TheNXLightFramePathParameter )
   {
      lightFrames[tableRow].path.Clear();
      if ( sizeOrLength > 0 )
         lightFrames[tableRow].path.SetLength( sizeOrLength );
   }
   else if ( p == TheNXFlatFramesParameter )
   {
      flatFrames.Clear();
      if ( sizeOrLength > 0 )
         flatFrames.Add( FrameItem(), sizeOrLength );
   }
   else if ( p == TheNXFlatFramePathParameter )
   {
      flatFrames[tableRow].path.Clear();
      if ( sizeOrLength > 0 )
         flatFrames[tableRow].path.SetLength( sizeOrLength );
   }
   else if ( p == TheNXCacheDirectoryParameter )
   {
      cacheDirectory.Clear();
      if ( sizeOrLength > 0 )
         cacheDirectory.SetLength( sizeOrLength );
   }
   else
      return false;

   return true;
}

size_type NukeXInstance::ParameterLength( const MetaParameter* p, size_type tableRow ) const
{
   if ( p == TheNXLightFramesParameter )       return lightFrames.Length();
   if ( p == TheNXLightFramePathParameter )    return lightFrames[tableRow].path.Length();
   if ( p == TheNXFlatFramesParameter )        return flatFrames.Length();
   if ( p == TheNXFlatFramePathParameter )     return flatFrames[tableRow].path.Length();
   if ( p == TheNXCacheDirectoryParameter )    return cacheDirectory.Length();
   return 0;
}

} // namespace pcl
