// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#ifndef __NukeXParameters_h
#define __NukeXParameters_h

#include <pcl/MetaParameter.h>

namespace pcl
{

// ── Input Light Frames Table ─────────────────────────────────────

class NXLightFrames : public MetaTable
{
public:
   NXLightFrames( MetaProcess* );
   IsoString Id() const override;
   size_type MinLength() const override;
};

class NXLightFramePath : public MetaString
{
public:
   NXLightFramePath( MetaTable* );
   IsoString Id() const override;
};

class NXLightFrameEnabled : public MetaBoolean
{
public:
   NXLightFrameEnabled( MetaTable* );
   IsoString Id() const override;
   bool DefaultValue() const override;
};

// ── Input Flat Frames Table ──────────────────────────────────────

class NXFlatFrames : public MetaTable
{
public:
   NXFlatFrames( MetaProcess* );
   IsoString Id() const override;
   size_type MinLength() const override;
};

class NXFlatFramePath : public MetaString
{
public:
   NXFlatFramePath( MetaTable* );
   IsoString Id() const override;
};

class NXFlatFrameEnabled : public MetaBoolean
{
public:
   NXFlatFrameEnabled( MetaTable* );
   IsoString Id() const override;
   bool DefaultValue() const override;
};

// ── Stretch Configuration ────────────────────────────────────────

class NXStretchType : public MetaEnumeration
{
public:
   NXStretchType( MetaProcess* );
   IsoString Id() const override;
   size_type NumberOfElements() const override;
   IsoString ElementId( size_type ) const override;
   int ElementValue( size_type ) const override;
   size_type DefaultValueIndex() const override;

   enum { VeraLux, GHS, MTF, ArcSinh, Log, Lupton,
          CLAHE, SAS, OTS, Photometric, NumberOfItems };
};

class NXAutoStretch : public MetaBoolean
{
public:
   NXAutoStretch( MetaProcess* );
   IsoString Id() const override;
   bool DefaultValue() const override;
};

// ── GPU Configuration ────────────────────────────────────────────

class NXEnableGPU : public MetaBoolean
{
public:
   NXEnableGPU( MetaProcess* );
   IsoString Id() const override;
   bool DefaultValue() const override;
};

// ── Cache Directory ──────────────────────────────────────────────

class NXCacheDirectory : public MetaString
{
public:
   NXCacheDirectory( MetaProcess* );
   IsoString Id() const override;
   String DefaultValue() const override;
};

// ── Global parameter pointers ────────────────────────────────────

extern NXLightFrames*      TheNXLightFramesParameter;
extern NXLightFramePath*   TheNXLightFramePathParameter;
extern NXLightFrameEnabled* TheNXLightFrameEnabledParameter;
extern NXFlatFrames*       TheNXFlatFramesParameter;
extern NXFlatFramePath*    TheNXFlatFramePathParameter;
extern NXFlatFrameEnabled* TheNXFlatFrameEnabledParameter;
extern NXStretchType*      TheNXStretchTypeParameter;
extern NXAutoStretch*      TheNXAutoStretchParameter;
extern NXEnableGPU*        TheNXEnableGPUParameter;
extern NXCacheDirectory*   TheNXCacheDirectoryParameter;

} // namespace pcl

#endif // __NukeXParameters_h
