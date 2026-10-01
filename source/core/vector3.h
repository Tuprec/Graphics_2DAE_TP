//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef VECTOR3_HEADER
#define VECTOR3_HEADER

//--- Standard Includes ---
#include <cstdint>
#include <limits>

namespace gfx
{
	struct Vector4;
	struct Vector2;
	struct Vector3 final
	{
		float x{};
		float y{};
		float z{};

		//--- Constructors & Destructors ---
		Vector3() = default;
		Vector3(const float _x, const float _y, const float _z);
		Vector3(const Vector4& v);
		~Vector3() = default;
		Vector3(const Vector3&) = default;
		Vector3& operator=(const Vector3&) = default;
		Vector3(Vector3&&) = default;
		Vector3& operator=(Vector3&&) = default;

		//--- Converters ---
		[[nodiscard]] Vector4 ToPoint4() const;
		[[nodiscard]] Vector4 ToVector4() const;
		[[nodiscard]] Vector2& AsVector2();

		//--- Functions ---
		[[nodiscard]] float Magnitude() const;
		[[nodiscard]] float SqrMagnitude() const;
		[[nodiscard]] Vector3 Normalized() const;
		float Normalize();

		static float Dot(const Vector3& v1, const Vector3& v2);
		static Vector3 Cross(const Vector3& v1, const Vector3& v2);
		static Vector4 Cross(const Vector4& v1, const Vector4& v2);
		static Vector3 Project(const Vector3& v1, const Vector3& v2);
		static Vector3 Reject(const Vector3& v1, const Vector3& v2);
		static Vector3 Reflect(const Vector3& v1, const Vector3& v2);
		static Vector3 Refract(const Vector3& incident, const Vector3& normal, 
			const float eta);

		static Vector3 Max(const Vector3& v1, const Vector3& v2);
		static Vector3 Min(const Vector3& v1, const Vector3& v2);

		//--- Operators ---
		Vector3 operator*(const float scale) const;
		Vector3 operator/(const float scale) const;
		Vector3 operator+(const Vector3& v) const;
		Vector3 operator-(const Vector3& v) const;
		Vector3 operator-() const;
		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3& operator/=(const float scale);
		Vector3& operator*=(const float scale);
		float& operator[](const uint8_t index);
		float operator[](const uint8_t index) const;
		bool operator==(const Vector3& v) const;
		bool operator!=(const Vector3& v) const;

		//--- Common Values ---
		static Vector3 UnitX() { return { 1, 0, 0 }; }
		static Vector3 UnitY() { return { 0, 1, 0 }; }
		static Vector3 UnitZ() { return { 0, 0, 1 }; }
		static Vector3 Zero() { return { 0, 0, 0 }; }
		static Vector3 Max() {
			return {
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max() }; }
	};

	//Global Operators
	inline Vector3 operator*(const float scale, const Vector3& v)
	{
		return { v.x * scale, v.y * scale, v.z * scale };
	}
}
#endif //VECTOR3_HEADER