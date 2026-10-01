//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef TEXTURE_HEADER
#define TEXTURE_HEADER

//--- Standard Includes ---
#include <vector>
#include <string>
#include <cmath>
#include <numbers>

//--- Framework Includes ---
#include <vector2.h>
#include <vector3.h>
#include <color.h>

namespace gfx
{
	//--- Class ----
	class Texture final
	{
		//--- Data members ---
		int width_{ 0 };
		int height_{ 0 };
		int channels_{ 0 };
		int pitch_{ 0 };
		bool is_srgb_{ false };
		std::vector<float> texel_data_{ };

	public:
		//--- Constructors & Destructor ---
		Texture(const std::string& path, bool is_srgb = false);
		~Texture() = default;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = default;
		Texture& operator=(Texture&&) = default;

		//--- Functions ---
		[[nodiscard]] ColorRgba Sample(const Vector2& uv) const;
		[[nodiscard]] Vector3 SampleNormal(const Vector2& uv, bool gltf_encoding = true) const;
		[[nodiscard]] ColorRgba SampleDirection(const Vector3& direction) const;

		[[nodiscard]] int GetWidth() const { return width_; }
		[[nodiscard]] int GetHeight() const { return height_; }
		[[nodiscard]] int GetChannels() const { return channels_; }
		[[nodiscard]] int GetPitch() const { return pitch_; }
		[[nodiscard]] const void* GetData() const { return texel_data_.data(); }
		[[nodiscard]] bool IsSrgb() const { return is_srgb_; }
	};
}
#endif //TEXTURE_HEADER