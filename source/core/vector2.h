//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef VECTOR2_HEADER
#define VECTOR2_HEADER

//--- Standard Includes ---
#include <cstdint>

namespace gfx
{
	struct Vector3;
	struct Vector4;
	struct Vector2 final
	{
		float x{};
		float y{};

		//--- Constructors & Destructors ---
		Vector2() = default;
		Vector2(const float _x, const float _y);
		Vector2(const Vector3& v);
		Vector2(const Vector4& v);
		~Vector2() = default;
		Vector2(const Vector2&) = default;
		Vector2& operator=(const Vector2&) = default;
		Vector2(Vector2&&) = default;
		Vector2& operator=(Vector2&&) = default;

		//--- Functions ---
		[[nodiscard]] float Magnitude() const;
		[[nodiscard]] float SqrMagnitude() const;
		[[nodiscard]] Vector2 Normalized() const;
		float Normalize();

		static float Dot(const Vector2& v1, const Vector2& v2);
		static float Cross(const Vector2& v1, const Vector2& v2);
		static float Cross(const Vector3& v1, const Vector3& v2);
		static float Cross(const Vector4& v1, const Vector4& v2);
		static Vector2 Reflect(const Vector2& v1, const Vector2& v2);

		static Vector2 Max(const Vector2& v1, const Vector2& v2);
		static Vector2 Min(const Vector2& v1, const Vector2& v2);
		static Vector2 Clamp(const Vector2& v, const float min = 0.f, const float max = 1.f);

		//--- Operators ---
		Vector2 operator*(const float scale) const;
		Vector2 operator/(const float scale) const;
		Vector2 operator+(const Vector2& v) const;
		Vector2 operator-(const Vector2& v) const;
		Vector2 operator-() const;
		Vector2& operator+=(const Vector2& v);
		Vector2& operator-=(const Vector2& v);
		Vector2& operator/=(const float scale);
		Vector2& operator*=(const float scale);
		float& operator[](const uint8_t index);
		float operator[](const uint8_t index) const;
		bool operator==(const Vector2& v) const;
		bool operator!=(const Vector2& v) const;

		//--- Common Values ---
		static Vector2 UnitX() { return { 1, 0 }; }
		static Vector2 UnitY() { return { 0, 1 }; }
		static Vector2 Zero() { return { 0, 0 }; }
	};

	//Global Operators
	inline Vector2 operator*(const float scale, const Vector2& v)
	{
		return { v.x * scale, v.y * scale };
	}
}
#endif //VECTOR2_HEADER