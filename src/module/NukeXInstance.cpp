// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXInstance.h"
#include "NukeXParameters.h"
#include "NukeXProcess.h"
#include "NukeXVersion.h"

#include "NukeXProgress.h"
#include <pcl/ImageWindow.h>
#include <pcl/View.h>
#include <pcl/FITSHeaderKeyword.h>

// NukeX pipeline headers
#include "nukex/stacker/stacking_engine.hpp"
#include "nukex/stretch/stretch_pipeline.hpp"
#include "fits_metadata.hpp"
#include "stretch_factory.hpp"

#include <sys/stat.h>
#include <unistd.h>

namespace {

// Build a FITS keyword set that records which build of NukeX produced this
// image and what the stacking result looked like.  Callers layer
// image-type-specific keywords (e.g., primaryStretch label) on top.
pcl::FITSKeywordArray base_output_keywords(
   const char* nukex_version,
   const char* image_kind,
   int n_frames_processed,
   int n_frames_failed_alignment )
{
   pcl::FITSKeywordArray ka;
   ka.Append( pcl::FITSHeaderKeyword(
      "CREATOR", pcl::IsoString( "'NukeX v" ) + nukex_version + "'",
      "Software that produced this image" ) );
   ka.Append( pcl::FITSHeaderKeyword(
      "NUKEXVER", pcl::IsoString( "'" ) + nukex_version + "'",
      "NukeX module version" ) );
   ka.Append( pcl::FITSHeaderKeyword(
      "NUKEXIMG", pcl::IsoString( "'" ) + image_kind + "'",
      "NukeX output kind: stacked | noise | stretched" ) );
   ka.Append( pcl::FITSHeaderKeyword(
      "NFRAMES", pcl::IsoString().Format( "%d", n_frames_processed ),
      "Number of light frames processed" ) );
   ka.Append( pcl::FITSHeaderKeyword(
      "NFAILALN", pcl::IsoString().Format( "%d", n_frames_failed_alignment ),
      "Frames flagged as failed alignment (weight x 0.5)" ) );
   ka.Append( pcl::FITSHeaderKeyword(
      "HISTORY", pcl::IsoString(),
      pcl::IsoString( "NukeX v" ) + nukex_version
          + " - Distribution-Fitted Stacking" ) );
   return ka;
}

} // anonymous namespace

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
   int n_enabled = 0;
   for ( const auto& f : lightFrames )
      if ( f.enabled ) ++n_enabled;
   if ( n_enabled == 0 )
   {
      whyNot = "No light frames are enabled.";
      return false;
   }

   // Cache directory writability check.  A non-writable cache is a
   // common misconfiguration (stale /tmp, wrong mount point, etc.)
   // and previously surfaced only as cryptic I/O errors deep inside
   // the ring-buffer later in Phase A.  Catch it at Validate time so
   // PI's Execute button stays disabled with a clear message.
   if ( !cacheDirectory.IsEmpty() )
   {
      String cache = cacheDirectory.Trimmed();
      if ( !cache.IsEmpty() )
      {
         IsoString cache_utf8 = cache.ToUTF8();
         const char* p = cache_utf8.c_str();
         struct stat st;
         if ( ::stat( p, &st ) != 0 || !S_ISDIR( st.st_mode ) )
         {
            whyNot = "Cache directory does not exist or is not a directory: " + cache;
            return false;
         }
         if ( ::access( p, W_OK ) != 0 )
         {
            whyNot = "Cache directory is not writable by the current user: " + cache;
            return false;
         }
      }
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
   int n_enabled = 0;
   for ( const auto& f : lightFrames )
      if ( f.enabled ) ++n_enabled;
   if ( n_enabled == 0 )
   {
      whyNot = "No light frames are enabled.";
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

   // Advisory: Student-t / contamination fitters need n ≥ 5 samples per
   // voxel to converge; with fewer frames, Phase B falls back to robust
   // stats only and distributional modelling is skipped.  Surface this
   // so users understand the quality cliff they're on.
   if ( light_paths.size() < 5 )
   {
      progress.message( String().Format(
         "** WARNING ** Only %zu enabled light frame(s).  Distribution "
         "fitting needs n >= 5; stacking will fall back to robust "
         "statistics per voxel and skip Student-t / contamination "
         "modelling.  Add more frames for full Phase B output quality.",
         light_paths.size() ).ToUTF8().c_str() );
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

   // Loud warnings on low alignment success rate.  Before v4.0.0.5,
   // a broken matcher silently weight-penalised 61/65 frames and the
   // only indicator was a buried summary line — the whole class of
   // "it ran, but most of my signal was at half weight" regression
   // can't be allowed to be quiet again.
   //
   // Thresholds:
   //   > 50% failed  → ** CRITICAL ** with remediation hint
   //   10–50% failed → ** WARNING  ** so users notice but can decide
   //   ≤ 10% failed  → no warning (some drift / clouded frames is normal)
   if ( result.n_frames_processed > 0 )
   {
      double pct = 100.0 * double( result.n_frames_failed_alignment )
                        / double( result.n_frames_processed );
      if ( pct > 50.0 )
      {
         progress.message( String().Format(
            "** CRITICAL ** %.1f%% of frames (%d of %d) failed alignment. "
            "Stacked result uses weight-penalised frames; SNR is significantly "
            "degraded.  Check that the first light frame is a reasonable "
            "reference (no clouds, well-tracked, enough stars), and that all "
            "lights cover a similar field.",
            pct, result.n_frames_failed_alignment,
            result.n_frames_processed ).ToUTF8().c_str() );
      }
      else if ( pct > 10.0 )
      {
         progress.message( String().Format(
            "** WARNING ** %.1f%% of frames (%d of %d) failed alignment and "
            "are stacked at 0.5x weight penalty.  Review the per-frame 'aligned'"
            " log lines above for which frames are affected.",
            pct, result.n_frames_failed_alignment,
            result.n_frames_processed ).ToUTF8().c_str() );
      }
   }

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

      // Provenance: stamp the FITS header so this window round-trips
      // through Save/Load with NukeX identity, version, and the run's
      // alignment-result counters intact.
      window.SetKeywords( base_output_keywords(
          NUKEX_VERSION_STRING, "stacked",
          result.n_frames_processed, result.n_frames_failed_alignment ) );

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
      // Provenance on the stretched window: include the primary +
      // finishing stretch enum labels so a user inspecting the saved
      // FITS later can tell which curve produced the image.
      pcl::FITSKeywordArray sw_ka = base_output_keywords(
          NUKEX_VERSION_STRING, "stretched",
          result.n_frames_processed, result.n_frames_failed_alignment );
      static const char* kPrimaryNames[] = {
          "Auto", "VeraLux", "GHS", "MTF", "ArcSinh", "Log", "Lupton", "CLAHE"
      };
      const int pe = static_cast<int>( primaryStretch );
      const int ne = static_cast<int>(
          sizeof(kPrimaryNames) / sizeof(kPrimaryNames[0]) );
      const char* primary_name =
          ( pe >= 0 && pe < ne ) ? kPrimaryNames[pe] : "Unknown";
      sw_ka.Append( pcl::FITSHeaderKeyword(
          "PRIMSTR", pcl::IsoString( "'" ) + primary_name + "'",
          "Primary stretch curve applied" ) );
      sw_ka.Append( pcl::FITSHeaderKeyword(
          "FINSTR", pcl::IsoString( "'None'" ),
          "Finishing stretch (only None enrolled today)" ) );
      sw_win.SetKeywords( sw_ka );

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

      nw.SetKeywords( base_output_keywords(
          NUKEX_VERSION_STRING, "noise",
          result.n_frames_processed, result.n_frames_failed_alignment ) );

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
