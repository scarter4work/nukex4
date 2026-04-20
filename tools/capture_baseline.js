// NukeX v4 — Phase B wall-time baseline capture harness.
// Invocation: PixInsight.sh --automation-mode --force-exit -r=tools/capture_baseline.js
// Env: NUKEX_BASELINE_STACK = directory containing FITS lights.
//
// NOTE on table column order:
//   NukeXProcess registers columns as [path, enabled] (NXLightFramePath first,
//   then NXLightFrameEnabled).  PJSR reflects that registration order, so each
//   row must be [path, enabled], NOT [enabled, path].
//   See src/module/NukeXProcess.cpp lines 20-22.

function getEnv(name) {
   try {
      if (typeof File.environmentVariable === "function")
         return File.environmentVariable(name) || "";
   } catch (e) {}
   return "";
}

function collectLights(dir) {
   var pats = ["*.fit", "*.fits", "*.FIT", "*.FITS"];
   var all = [];
   for (var i = 0; i < pats.length; i++) {
      var found = searchDirectory(dir + "/" + pats[i], false);
      for (var j = 0; j < found.length; j++) all.push(found[j]);
   }
   return all;
}

function main() {
   var stackDir = getEnv("NUKEX_BASELINE_STACK");
   if (!stackDir) {
      Console.criticalln("NUKEX_BASELINE_STACK not set");
      throw new Error("missing env var");
   }

   var lights = collectLights(stackDir);
   if (lights.length === 0) {
      Console.criticalln("No FITS found in " + stackDir);
      throw new Error("no lights");
   }
   Console.writeln("Baseline dir: " + stackDir);
   Console.writeln("Baseline lights: " + lights.length);

   var P = new NukeX;

   // Column order matches NukeXProcess registration: [path, enabled]
   // NXLightFramePath is registered first, NXLightFrameEnabled second.
   var arr = [];
   for (var i = 0; i < lights.length; i++) arr.push([lights[i], true]);
   P.lightFrames = arr;
   P.flatFrames = [];
   P.primaryStretch = NukeX.prototype.primaryStretch_Auto;
   P.finishingStretch = NukeX.prototype.finishingStretch_None;
   P.enableGPU = true;
   P.cacheDirectory = "/tmp";

   var start = new Date().getTime();
   var ok = P.executeGlobal();
   var elapsed = new Date().getTime() - start;
   Console.writeln("TOTAL_MS " + elapsed);
   if (!ok) {
      Console.criticalln("executeGlobal returned false");
      throw new Error("execute failed");
   }
}

main();
