// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#ifndef __NukeXProgress_h
#define __NukeXProgress_h

#include <pcl/Console.h>
#include <pcl/StandardStatus.h>
#include <pcl/StatusMonitor.h>

#include "nukex/core/progress_observer.hpp"
#include <vector>
#include <string>

namespace pcl
{

/// Bridges nukex::ProgressObserver to PI's Console + StatusMonitor.
/// The outermost begin_phase() drives the progress bar.
/// Inner phases and advance() detail strings write indented text to Console.
class NukeXProgress : public nukex::ProgressObserver
{
public:
   NukeXProgress();

   void begin_phase( const std::string& name, int total_steps ) override;
   void advance( int steps, const std::string& detail ) override;
   void end_phase() override;
   void message( const std::string& text ) override;
   bool is_cancelled() const override;

private:
   struct Phase {
      std::string name;
      int total;
      int current;
   };

   Console            console_;
   StandardStatus     status_;
   StatusMonitor      monitor_;
   std::vector<Phase> stack_;
   int                depth_ = 0;  // nesting depth for indentation
};

} // namespace pcl

#endif // __NukeXProgress_h
