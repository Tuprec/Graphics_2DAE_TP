//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef SOFTWARE_RASTERIZER_HEADER
#define SOFTWARE_RASTERIZER_HEADER

//--- Standard Includes ---

//--- Framework Includes ---
#include <renderer.h>

namespace gfx
{
	//--- Software Rasterizer ---
	class SoftwareRasterizer final : public Renderer
	{
	public:
		//--- Construction / Destruction ---
		SoftwareRasterizer(Context* const context);
		~SoftwareRasterizer() override;

		SoftwareRasterizer(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer& operator=(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer(SoftwareRasterizer&&) = delete;
		SoftwareRasterizer& operator=(SoftwareRasterizer&&) = delete;

		//--- Public Functions ---
		void Render() override;
	};
}
#endif //SOFTWARE_RASTERIZER_HEADER
