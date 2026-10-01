//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef COLOR_HEADER
#define COLOR_HEADER

//--- Standard includes ---
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

//--- Framework includes ---
#include <vector2.h>
#include <vector3.h>
#include <vector4.h>

namespace gfx
{
	enum class ToneMapOperator : uint8_t
	{
		kNone,
		kReinhard,
		kACESNarkowicz,
		kACESHill,
		kMax
	};

	struct ColorRgba final
	{
		float r{ 0.f };
		float g{ 0.f };
		float b{ 0.f };
		float a{ 1.f };

		//--- Supporting Data ---
		enum class ConvertiblePixelFormat : uint8_t
		{
			ARGB8888,
			RGBA8888,
			ABGR8888,
			BGRA8888
		};

		//--- Constructors & Destructors ---
		ColorRgba(const float v) : r(v), g(v), b(v), a(1.f) {}
		ColorRgba(const float r, const float g, const float b) : r(r), g(g), b(b), a(1.f) {}
		ColorRgba(const float r, const float g, const float b, const float a) : r(r), g(g), b(b), a(a) {}
		ColorRgba(const std::array<const float, 3>& v) : r(v[0]), g(v[1]), b(v[2]), a(1.f) {}
		ColorRgba(const std::array<const float, 4>& v) : r(v[0]), g(v[1]), b(v[2]), a(v[3]) {}
		ColorRgba(const Vector2& v) : r(v[0]), g(v[1]), b(0.f), a(1.f) {}
		ColorRgba(const Vector3& v) : r(v[0]), g(v[1]), b(v[2]), a(1.f) {}
		ColorRgba(const Vector4& v) : r(v[0]), g(v[1]), b(v[2]), a(v[3]) {}
		ColorRgba() = default;
		~ColorRgba() = default;
		ColorRgba(const ColorRgba&) = default;
		ColorRgba& operator=(const ColorRgba&) = default;
		ColorRgba(ColorRgba&&) = default;
		ColorRgba& operator=(ColorRgba&&) = default;

		//--- Functions ---
		void Clamp()
		{
			r = std::max(0.0f, std::min(r, 1.0f));
			g = std::max(0.0f, std::min(g, 1.0f));
			b = std::max(0.0f, std::min(b, 1.0f));
			a = std::max(0.0f, std::min(a, 1.0f));
		}

		void MaxToOne()
		{
			const float alpha{ a };
			const float max_value{ std::max(r, std::max(g, b)) };
			if (max_value > 1.0f)
				*this /= max_value;
			this->a = std::max(std::min(alpha, 1.f), 0.f);
		}

		[[nodiscard]] ColorRgba TonemapReinhard() const
		{
			// Luminance-based Reinhard: preserves color ratios (saturation)
			const float lum{ Luminance() };
			if (lum <= 0.f) return { 0.f, 0.f, 0.f, a };
			const float mapped_lum{ lum / (1.f + lum) };
			const float scale{ mapped_lum / lum };
			return ColorRgba(r * scale, g * scale, b * scale, a);
		}

		[[nodiscard]] ColorRgba TonemapACESNarkowicz() const
		{
			// Simple per-channel approximation, no color space conversion.
			auto aces = [](const float x) -> float
				{
					constexpr float a{ 2.51f };
					constexpr float b{ 0.03f };
					constexpr float c{ 2.43f };
					constexpr float d{ 0.59f };
					constexpr float e{ 0.14f };
					return std::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.f, 1.f);
				};
			return ColorRgba(aces(r), aces(g), aces(b), a);
		}

		[[nodiscard]] ColorRgba TonemapACESHill() const
		{
			// sRGB -> ACES AP1 input matrix
			const float ir{ 0.59719f * r + 0.35458f * g + 0.04823f * b };
			const float ig{ 0.07600f * r + 0.90834f * g + 0.01566f * b };
			const float ib{ 0.02840f * r + 0.13383f * g + 0.83777f * b };

			// RRT + ODT fitted curve
			auto rrt_odt = [](const float v) -> float
				{
					const float va{ v * (v + 0.0245786f) - 0.000090537f };
					const float vb{ v * (0.983729f * v + 0.4329510f) + 0.238081f };
					return va / vb;
				};
			const float mr{ rrt_odt(ir) };
			const float mg{ rrt_odt(ig) };
			const float mb{ rrt_odt(ib) };

			// ACES AP1 -> sRGB output matrix
			return ColorRgba(
				std::clamp(1.60475f * mr - 0.53108f * mg - 0.07367f * mb, 0.f, 1.f),
				std::clamp(-0.10208f * mr + 1.10813f * mg - 0.00605f * mb, 0.f, 1.f),
				std::clamp(-0.00327f * mr - 0.07276f * mg + 1.07602f * mb, 0.f, 1.f),
				a);
		}

		[[nodiscard]] ColorRgba Tonemap(const ToneMapOperator op) const
		{
			switch (op)
			{
			case ToneMapOperator::kReinhard: return TonemapReinhard();
			case ToneMapOperator::kACESNarkowicz: return TonemapACESNarkowicz();
			case ToneMapOperator::kACESHill: return TonemapACESHill();
			default: return *this;
			}
		}

		[[nodiscard]] ColorRgba ToLinear() const
		{
			const float linear_r{ (r <= 0.04045f) ? r / 12.92f : std::pow((r + 0.055f) / 1.055f, 2.4f) };
			const float linear_g{ (g <= 0.04045f) ? g / 12.92f : std::pow((g + 0.055f) / 1.055f, 2.4f) };
			const float linear_b{ (b <= 0.04045f) ? b / 12.92f : std::pow((b + 0.055f) / 1.055f, 2.4f) };
			return ColorRgba(linear_r, linear_g, linear_b, a);
		}

		[[nodiscard]] ColorRgba ToSrgb() const
		{
			const float srgb_r{ (r <= 0.0031308f) ? r * 12.92f : 1.055f * std::pow(r, 1.0f / 2.4f) - 0.055f };
			const float srgb_g{ (g <= 0.0031308f) ? g * 12.92f : 1.055f * std::pow(g, 1.0f / 2.4f) - 0.055f };
			const float srgb_b{ (b <= 0.0031308f) ? b * 12.92f : 1.055f * std::pow(b, 1.0f / 2.4f) - 0.055f };
			return ColorRgba(srgb_r, srgb_g, srgb_b, a);
		}

		[[nodiscard]] ColorRgba ToGamma(const float gamma = 2.2f) const
		{
			const float inv_gamma{ 1.f / gamma };
			return ColorRgba(std::pow(r, inv_gamma), std::pow(g, inv_gamma), std::pow(b, inv_gamma), a);
		}

		[[nodiscard]] float Luminance() const
		{
			return 0.2126f * r + 0.7152f * g + 0.0722f * b;
		}

		[[nodiscard]] ColorRgba Lerp(const ColorRgba& other, const float t) const
		{
			return ColorRgba(
				r + t * (other.r - r),
				g + t * (other.g - g),
				b + t * (other.b - b),
				a + t * (other.a - a));
		}

		[[nodiscard]] static ColorRgba Lerp(const ColorRgba& c1, const ColorRgba& c2, const float t)
		{
			return c1.Lerp(c2, t);
		}

		[[nodiscard]] uint32_t ToUint32(
			const ConvertiblePixelFormat format = ConvertiblePixelFormat::RGBA8888,
			const bool convert_to_srgb = false) const
		{
			ColorRgba color = convert_to_srgb ? ToSrgb() : *this;
			color.Clamp();

			const uint8_t red{ static_cast<uint8_t>(color.r * 255.0f) };
			const uint8_t green{ static_cast<uint8_t>(color.g * 255.0f) };
			const uint8_t blue{ static_cast<uint8_t>(color.b * 255.0f) };
			const uint8_t alpha{ static_cast<uint8_t>(color.a * 255.0f) };

			switch (format)
			{
			case ConvertiblePixelFormat::ARGB8888: return (alpha << 24) | (red << 16) | (green << 8) | blue;
			case ConvertiblePixelFormat::RGBA8888: return (red << 24) | (green << 16) | (blue << 8) | alpha;
			case ConvertiblePixelFormat::ABGR8888: return (alpha << 24) | (blue << 16) | (green << 8) | red;
			case ConvertiblePixelFormat::BGRA8888: return (blue << 24) | (green << 16) | (red << 8) | alpha;
			}
			return 0;
		}

		//--- Operators ---
		ColorRgba& operator+=(const ColorRgba& o)
		{
			r += o.r; g += o.g; b += o.b; a += o.a;
			return *this;
		}

		ColorRgba operator+(const ColorRgba& o) const
		{
			return ColorRgba(r + o.r, g + o.g, b + o.b, a + o.a);
		}

		ColorRgba& operator-=(const ColorRgba& o)
		{
			r -= o.r; g -= o.g; b -= o.b; a -= o.a;
			return *this;
		}

		ColorRgba operator-(const ColorRgba& o) const
		{
			return ColorRgba(r - o.r, g - o.g, b - o.b, a - o.a);
		}

		ColorRgba& operator*=(const float s)
		{
			r *= s; g *= s; b *= s; a *= s;
			return *this;
		}

		ColorRgba operator*(const float s) const
		{
			return ColorRgba(r * s, g * s, b * s, a * s);
		}

		ColorRgba& operator/=(float s)
		{
			if (s != 0.0f) { r /= s; g /= s; b /= s; a /= s; }
			return *this;
		}

		ColorRgba operator/(float s) const
		{
			return (s != 0.0f) ? ColorRgba(r / s, g / s, b / s, a / s) : *this;
		}

		ColorRgba& operator*=(const ColorRgba& o)
		{
			r *= o.r; g *= o.g; b *= o.b; a *= o.a;
			return *this;
		}

		ColorRgba operator*(const ColorRgba& o) const
		{
			return ColorRgba(r * o.r, g * o.g, b * o.b, a * o.a);
		}

		friend ColorRgba operator*(float s, const ColorRgba& col) { return col * s; }

		bool operator==(const ColorRgba& o) const
		{
			auto eq = [](const float x, const float y)
				{
					const float diff{ std::fabs(x - y) };
					const float scale{ std::max({ 1.0f, std::fabs(x), std::fabs(y) }) };
					return diff <= std::numeric_limits<float>::epsilon() * scale;
				};
			return eq(r, o.r) && eq(g, o.g) && eq(b, o.b) && eq(a, o.a);
		}

		bool operator!=(const ColorRgba& o) const
		{
			return !(*this == o);
		}

		//--- Common Values ---
		static ColorRgba Red() { return { 1, 0, 0 }; }
		static ColorRgba Green() { return { 0, 1, 0 }; }
		static ColorRgba Blue() { return { 0, 0, 1 }; }
		static ColorRgba Yellow() { return { 1, 1, 0 }; }
		static ColorRgba Cyan() { return { 0, 1, 1 }; }
		static ColorRgba Magenta() { return { 1, 0, 1 }; }
		static ColorRgba White() { return { 1, 1, 1 }; }
		static ColorRgba Black() { return { 0, 0, 0 }; }
		static ColorRgba Gray() { return { .5f, .5f, .5f }; }
	};
}
#endif //COLOR_HEADER