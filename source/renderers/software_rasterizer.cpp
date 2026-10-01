//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <software_rasterizer.h>
#include <scenes.h>
using namespace gfx;

// =============================================================================
// Construction / Destruction
// =============================================================================
SoftwareRasterizer::SoftwareRasterizer(Context* const context)
	: Renderer(context)
{}

SoftwareRasterizer::~SoftwareRasterizer() = default;

// =============================================================================
// Public Functions
// =============================================================================
void SoftwareRasterizer::Render()
{
	assert(context_ && "Context not available!");

	// DEMO CODE - TODO: remove!
	const SurfaceInfo& surface_info = context_->surface_info;
	for (uint32_t py = 0; py < surface_info.height; ++py)
	{
		for (uint32_t px = 0; px < surface_info.width; ++px)
		{
			// Calculate a gradient value based on pixel screen coordinates.
			float gradient = px / static_cast<float>(surface_info.width);
			gradient += py / static_cast<float>(surface_info.height);
			gradient /= 2.0f;

			// Convert gradient value to color.
			ColorRgba final_color = { gradient, gradient, gradient };
			final_color.MaxToOne();

			// Write to surface
			surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
				surface_info.pixel_format_details, nullptr,
				static_cast<uint8_t>(final_color.r * 255),
				static_cast<uint8_t>(final_color.g * 255),
				static_cast<uint8_t>(final_color.b * 255));
		}
	}
}