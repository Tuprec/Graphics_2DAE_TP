//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
using namespace gfx;

Texture::Texture(const std::string& path, const bool is_srgb)
	: is_srgb_(is_srgb)
{
	if (stbi_is_hdr(path.c_str()))
	{
		float* raw_data = stbi_loadf(path.c_str(), &width_, &height_, &channels_, 0);
		if (raw_data)
		{
			texel_data_.assign(raw_data, raw_data + width_ * height_ * channels_);
			pitch_ = width_ * channels_;
			stbi_image_free(raw_data);
		}
	}
	else
	{
		unsigned char* raw_data = stbi_load(path.c_str(), &width_, &height_, &channels_, 0);
		if (raw_data)
		{
			const int total{ width_ * height_ * channels_ };
			texel_data_.resize(total);
			for (int i = 0; i < total; ++i)
				texel_data_[i] = static_cast<float>(raw_data[i]) / 255.f;
			pitch_ = width_ * channels_;
			stbi_image_free(raw_data);
		}
	}
}

ColorRgba Texture::Sample(const Vector2& uv) const
{
	// Calculate index of texel.
	const Vector2 clamped_uv{ Vector2::Clamp(uv) };
	const int texel_x{ std::min(static_cast<int>(clamped_uv.x * 
		static_cast<float>(width_)), width_ - 1) };
	const int texel_y{ std::min(static_cast<int>(clamped_uv.y * 
		static_cast<float>(height_)), height_ - 1) };
	const int index{ texel_y * pitch_ + texel_x * channels_ };

	// Read channels (already stored as float).
	float r{ 0 }, g{ 0 }, b{ 0 }, a{ 1.0f };
	if (channels_ >= 1) r = texel_data_[index];
	if (channels_ >= 2) g = texel_data_[index + 1];
	if (channels_ >= 3) b = texel_data_[index + 2];
	if (channels_ == 4) a = texel_data_[index + 3];
	return { r, g, b, a };
}

Vector3 Texture::SampleNormal(const Vector2& uv, bool gltf_encoding) const
{
	// Calculate index of texel.
	const Vector2 clamped_uv{ Vector2::Clamp(uv) };
	const int texel_x{ std::min(static_cast<int>(clamped_uv.x * 
		static_cast<float>(width_)), width_ - 1) };
	const int texel_y{ std::min(static_cast<int>(clamped_uv.y * 
		static_cast<float>(height_)), height_ - 1) };
	const int index{ texel_y * pitch_ + texel_x * channels_ };

	// Read channels (already stored as float, in [0,1] for LDR).
	float nx{ 0.0f }, ny{ 0.0f }, nz{ 1.0f };
	if (channels_ >= 1) nx = texel_data_[index];
	if (channels_ >= 2) ny = texel_data_[index + 1];
	if (channels_ >= 3) nz = texel_data_[index + 2];

	// Remap based on setup.
	if (gltf_encoding)
	{
		// glTF tangent-space normal remapping.
		// See: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#additional-textures
		nx = nx * 2.0f - 1.0f;
		ny = ny * 2.0f - 1.0f;
		nz = std::sqrt(std::max(0.f, 1.0f - nx * nx - ny * ny));
	}
	else
	{
		// Generic normal map [-1,1] mapping.
		nx = nx * 2.0f - 1.0f;
		ny = ny * 2.0f - 1.0f;
		nz = nz * 2.0f - 1.0f;
	}

	// Normalize the vector to be safe.
	return Vector3(nx, ny, nz).Normalized();
}

ColorRgba Texture::SampleDirection(const Vector3& direction) const
{
	// Equirectangular mapping: direction -> UV
	// atan2(x, z) matches the standard convention (Blender, Poly Haven, etc.):
	// u=0.5 (center) = +Z forward, increasing u = rotating right (clockwise from above)
	const float u{ 0.5f - std::atan2(direction.x, direction.z)
		/ (2.f * static_cast<float>(std::numbers::pi)) };
	const float v{ 0.5f - std::asin(std::clamp(direction.y, -1.f, 1.f))
		/ static_cast<float>(std::numbers::pi) };
	return Sample({ u, v });
}
