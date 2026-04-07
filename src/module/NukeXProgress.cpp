// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXProgress.h"
#include <pcl/String.h>

namespace pcl
{

NukeXProgress::NukeXProgress()
   : status_()
   , monitor_()
{
}

void NukeXProgress::begin_phase( const std::string& name, int total_steps )
{
   depth_++;
   stack_.push_back( { name, total_steps, 0 } );

   if ( depth_ == 1 )
   {
      // Outermost phase drives the progress bar
      console_.WriteLn( String( "<end><cbr>" ) );
      console_.WriteLn( String().Format( "\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90 %s (%d steps) \xe2\x95\x90\xe2\x95\x90\xe2\x95\x90",
                                         name.c_str(), total_steps ) );
      monitor_.SetCallback( &status_ );
      monitor_.Initialize( String( name.c_str() ), total_steps );
   }
   else
   {
      // Nested phase — console only
      String indent;
      for ( int i = 1; i < depth_; i++ ) indent += "  ";
      console_.WriteLn( indent + String( name.c_str() ) );
   }
   console_.Flush();
}

void NukeXProgress::advance( int steps, const std::string& detail )
{
   if ( stack_.empty() ) return;

   auto& phase = stack_.back();

   if ( !detail.empty() )
   {
      String indent;
      for ( int i = 0; i < depth_; i++ ) indent += "  ";
      console_.WriteLn( indent + String( detail.c_str() ) );
      console_.Flush();
   }

   if ( steps > 0 )
   {
      phase.current += steps;

      // Advance the outermost progress bar
      if ( depth_ == 1 )
      {
         for ( int i = 0; i < steps; i++ )
            ++monitor_;
      }
   }
}

void NukeXProgress::end_phase()
{
   if ( stack_.empty() ) return;

   if ( depth_ == 1 )
   {
      // Close the progress bar
      monitor_.Complete();
   }

   stack_.pop_back();
   depth_--;
}

void NukeXProgress::message( const std::string& text )
{
   console_.WriteLn( String( text.c_str() ) );
   console_.Flush();
}

bool NukeXProgress::is_cancelled() const
{
   return monitor_.IsAborted();
}

} // namespace pcl
