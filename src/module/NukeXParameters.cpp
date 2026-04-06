// NukeX v4 — Distribution-Fitted Stacking for PixInsight
// Copyright (c) 2026 Scott Carter. MIT License.

#include "NukeXParameters.h"

namespace pcl
{

// ── Global parameter pointers ────────────────────────────────────

NXLightFrames*       TheNXLightFramesParameter = nullptr;
NXLightFramePath*    TheNXLightFramePathParameter = nullptr;
NXLightFrameEnabled* TheNXLightFrameEnabledParameter = nullptr;
NXFlatFrames*        TheNXFlatFramesParameter = nullptr;
NXFlatFramePath*     TheNXFlatFramePathParameter = nullptr;
NXFlatFrameEnabled*  TheNXFlatFrameEnabledParameter = nullptr;
NXStretchType*       TheNXStretchTypeParameter = nullptr;
NXAutoStretch*       TheNXAutoStretchParameter = nullptr;
NXEnableGPU*         TheNXEnableGPUParameter = nullptr;
NXCacheDirectory*    TheNXCacheDirectoryParameter = nullptr;

// ── Light Frames Table ───────────────────────────────────────────

NXLightFrames::NXLightFrames( MetaProcess* p ) : MetaTable( p )
{
   TheNXLightFramesParameter = this;
}

IsoString NXLightFrames::Id() const { return "lightFrames"; }
size_type NXLightFrames::MinLength() const { return 0; }

NXLightFramePath::NXLightFramePath( MetaTable* t ) : MetaString( t )
{
   TheNXLightFramePathParameter = this;
}

IsoString NXLightFramePath::Id() const { return "path"; }

NXLightFrameEnabled::NXLightFrameEnabled( MetaTable* t ) : MetaBoolean( t )
{
   TheNXLightFrameEnabledParameter = this;
}

IsoString NXLightFrameEnabled::Id() const { return "enabled"; }
bool NXLightFrameEnabled::DefaultValue() const { return true; }

// ── Flat Frames Table ────────────────────────────────────────────

NXFlatFrames::NXFlatFrames( MetaProcess* p ) : MetaTable( p )
{
   TheNXFlatFramesParameter = this;
}

IsoString NXFlatFrames::Id() const { return "flatFrames"; }
size_type NXFlatFrames::MinLength() const { return 0; }

NXFlatFramePath::NXFlatFramePath( MetaTable* t ) : MetaString( t )
{
   TheNXFlatFramePathParameter = this;
}

IsoString NXFlatFramePath::Id() const { return "path"; }

NXFlatFrameEnabled::NXFlatFrameEnabled( MetaTable* t ) : MetaBoolean( t )
{
   TheNXFlatFrameEnabledParameter = this;
}

IsoString NXFlatFrameEnabled::Id() const { return "enabled"; }
bool NXFlatFrameEnabled::DefaultValue() const { return true; }

// ── Stretch Configuration ────────────────────────────────────────

NXStretchType::NXStretchType( MetaProcess* p ) : MetaEnumeration( p )
{
   TheNXStretchTypeParameter = this;
}

IsoString NXStretchType::Id() const { return "stretchType"; }

size_type NXStretchType::NumberOfElements() const
{
   return NumberOfItems;
}

IsoString NXStretchType::ElementId( size_type i ) const
{
   switch ( i )
   {
   case VeraLux:      return "VeraLux";
   case GHS:          return "GHS";
   case MTF:          return "MTF";
   case ArcSinh:      return "ArcSinh";
   case Log:          return "Log";
   case Lupton:       return "Lupton";
   case CLAHE:        return "CLAHE";
   case SAS:          return "SAS";
   case OTS:          return "OTS";
   case Photometric:  return "Photometric";
   default:           return IsoString();
   }
}

int NXStretchType::ElementValue( size_type i ) const
{
   return int( i );  // Values match enum indices
}

size_type NXStretchType::DefaultValueIndex() const
{
   return VeraLux;  // Overall champion from optimization
}

NXAutoStretch::NXAutoStretch( MetaProcess* p ) : MetaBoolean( p )
{
   TheNXAutoStretchParameter = this;
}

IsoString NXAutoStretch::Id() const { return "autoStretch"; }
bool NXAutoStretch::DefaultValue() const { return true; }

// ── GPU Configuration ────────────────────────────────────────────

NXEnableGPU::NXEnableGPU( MetaProcess* p ) : MetaBoolean( p )
{
   TheNXEnableGPUParameter = this;
}

IsoString NXEnableGPU::Id() const { return "enableGPU"; }
bool NXEnableGPU::DefaultValue() const { return true; }

// ── Cache Directory ──────────────────────────────────────────────

NXCacheDirectory::NXCacheDirectory( MetaProcess* p ) : MetaString( p )
{
   TheNXCacheDirectoryParameter = this;
}

IsoString NXCacheDirectory::Id() const { return "cacheDirectory"; }
String NXCacheDirectory::DefaultValue() const { return "/tmp"; }

} // namespace pcl
