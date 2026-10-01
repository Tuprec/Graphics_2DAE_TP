//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <hardware_rasterizer.h>
#include <scenes.h>
using namespace gfx;

// =============================================================================
// Construction / Destruction
// =============================================================================
HardwareRasterizer::HardwareRasterizer(Context* const context)
	: Renderer(context)
{}

HardwareRasterizer::~HardwareRasterizer() = default;

// =============================================================================
// Public Functions
// =============================================================================
void HardwareRasterizer::Render()
{
	assert(context_ && "Context not available!");
}