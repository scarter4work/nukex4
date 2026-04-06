// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXInstance.h"
#include "NukeXParameters.h"
#include "NukeXProcess.h"

#include <pcl/Console.h>
#include <pcl/StandardStatus.h>
#include <pcl/ImageWindow.h>
#include <pcl/View.h>

// NukeX pipeline headers
#include "nukex/stacker/stacking_engine.hpp"

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
      stretchType    = x->stretchType;
      autoStretch    = x->autoStretch;
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
   Console console;
   console.WriteLn( "<end><cbr><br>NukeX v4 — Distribution-Fitted Stacking" );
   console.WriteLn( String().Format( "Processing %zu light frame(s)", lightFrames.Length() ) );

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
      console.WriteLn( "** No enabled light frames. Aborting." );
      return false;
   }

   console.WriteLn( String().Format( "Light frames: %zu enabled", light_paths.size() ) );
   if ( !flat_paths.empty() )
      console.WriteLn( String().Format( "Flat frames: %zu enabled", flat_paths.size() ) );

   // Configure stacking engine
   nukex::StackingEngine::Config config;
   config.cache_dir = cacheDirectory.ToUTF8().c_str();
   config.gpu_config.force_cpu_fallback = !enableGPU;

   // Execute pipeline
   console.WriteLn( "Starting stacking pipeline..." );
   nukex::StackingEngine engine( config );
   auto result = engine.execute( light_paths, flat_paths );

   if ( result.n_frames_processed == 0 )
   {
      console.WriteLn( "** No frames were processed. Check input files." );
      return false;
   }

   console.WriteLn( String().Format(
      "Stacking complete: %d frame(s) processed, %d failed alignment",
      result.n_frames_processed, result.n_frames_failed_alignment ) );

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
                          true,  // initialProcessing (records NukeX as creator)
                          "NukeX_stacked" );

      View view = window.MainView();
      ImageVariant v = view.Image();

      // Copy stacked data to the PI image.
      // We created a 32-bit float window, so the image is pcl::Image (Float32).
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
      console.WriteLn( "Stacked image opened." );
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
      console.WriteLn( "Noise map opened." );
   }

   console.WriteLn( "<br>NukeX v4 done." );
   return true;
}

// ── Parameter serialization ──────────────────────────────────────

void* NukeXInstance::LockParameter( const MetaParameter* p, size_type tableRow )
{
   if ( p == TheNXLightFramePathParameter )    return lightFrames[tableRow].path.Begin();
   if ( p == TheNXLightFrameEnabledParameter ) return &lightFrames[tableRow].enabled;
   if ( p == TheNXFlatFramePathParameter )     return flatFrames[tableRow].path.Begin();
   if ( p == TheNXFlatFrameEnabledParameter )  return &flatFrames[tableRow].enabled;
   if ( p == TheNXStretchTypeParameter )       return &stretchType;
   if ( p == TheNXAutoStretchParameter )       return &autoStretch;
   if ( p == TheNXEnableGPUParameter )         return &enableGPU;
   if ( p == TheNXCacheDirectoryParameter )    return cacheDirectory.Begin();
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
