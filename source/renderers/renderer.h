//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef RENDERER_HEADER
#define RENDERER_HEADER

//--- Framework Includes ---
#include <context.h>

namespace gfx
{
	//--- Base Renderer ---
	class Renderer
	{
	protected:
		//--- Protected Members ---
		Context* context_{ nullptr };

	public:
		//--- Construction / Destruction ---
		Renderer(Context* const context) : context_(context) {}
		virtual ~Renderer() = default;
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		//--- Pure Virtual Functions ---
		virtual void Render() = 0;
	};
}
#endif //RENDERER_HEADER
