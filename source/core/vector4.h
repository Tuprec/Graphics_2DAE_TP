//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef VECTOR4_HEADER
#define VECTOR4_HEADER

//--- Standard Includes ---
#include <cstdint>

namespace gfx
{
	struct Vector2;
	struct Vector3;
	struct Vector4 final
	{
		float x{};
		float y{};
		float z{};
		float w{};

		//--- Constructors & Destructors ---
		Vector4() = default;
		Vector4(float _x, float _y, float _z, float _w);
		Vector4(const Vector3& v, float _w);
		~Vector4() = default;
		Vector4(const Vector4&) = default;
		Vector4& operator=(const Vector4&) = default;
		Vector4(Vector4&&) = default;
		Vector4& operator=(Vector4&&) = default;

		//--- Converters ---
		[[nodiscard]] Vector3& AsVector3();
		[[nodiscard]] Vector2& AsVector2();

		//--- Functions ---
		[[nodiscard]] float Magnitude() const;
		[[nodiscard]] float SqrMagnitude() const;
		[[nodiscard]] Vector4 Normalized() const;
		float Normalize();

		static float Dot(const Vector4& v1, const Vector4& v2);

		//--- Operators ---
		Vector4 operator*(const float scale) const;
		Vector4 operator/(const float scale) const;
		Vector4 operator+(const Vector4& v) const;
		Vector4 operator-(const Vector4& v) const;
		Vector4 operator-() const;
		Vector4& operator*=(const float scale);
		Vector4& operator/=(const float scale);
		Vector4& operator+=(const Vector4& v);
		Vector4& operator-=(const Vector4& v);
		float& operator[](const uint8_t index);
		float operator[](const uint8_t index) const;
		bool operator==(const Vector4& v) const;
		bool operator!=(const Vector4& v) const;

		//--- Common Values ---
		static Vector4 UnitX() { return { 1, 0, 0, 0 }; }
		static Vector4 UnitY() { return { 0, 1, 0, 0 }; }
		static Vector4 UnitZ() { return { 0, 0, 1, 0 }; }
		static Vector4 UnitW() { return { 0, 0, 0, 1 }; }
		static Vector4 Zero() { return { 0, 0, 0, 0 }; }
	};
}
#endif //VECTOR4_HEADER
