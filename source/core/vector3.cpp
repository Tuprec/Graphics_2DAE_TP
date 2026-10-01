//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <vector3.h>
#include <vector4.h>
#include <vector2.h>
#include <algorithm>
#include <cmath>
using namespace gfx;

//--- Standard Includes ---
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>

Vector3::Vector3(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z){}

Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z){}

Vector4 Vector3::ToPoint4() const
{
	return { x, y, z, 1 };
}

Vector4 Vector3::ToVector4() const
{
	return { x, y, z, 0 };
}

Vector2& Vector3::AsVector2()
{
	static_assert(offsetof(Vector3, x) == offsetof(Vector2, x)
		&& offsetof(Vector3, y) == offsetof(Vector2, y),
		"Layout mismatch between Vector3 and Vector2");
	return reinterpret_cast<Vector2&>(*this);
}

float Vector3::Magnitude() const
{
	return std::sqrt(x * x + y * y + z * z);
}

float Vector3::SqrMagnitude() const
{
	return x * x + y * y + z * z;
}

float Vector3::Normalize()
{
	const float m{ Magnitude() };
	if (m > 0.f) { x /= m; y /= m; z /= m; }
	return m;
}

Vector3 Vector3::Normalized() const
{
	const float m{ Magnitude() };
	if (m > 0.f) return { x / m, y / m, z / m };
	return {};
}

float Vector3::Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x };
}

Vector4 Vector3::Cross(const Vector4& v1, const Vector4& v2)
{
	return Vector4{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x, 0.f };
}

Vector3 Vector3::Project(const Vector3& v1, const Vector3& v2)
{
	return (v2 * (Dot(v1, v2) / Dot(v2, v2))); 
}

Vector3 Vector3::Reject(const Vector3& v1, const Vector3& v2)
{
	return (v1 - v2 * (Dot(v1, v2) / Dot(v2, v2)));
}

Vector3 Vector3::Reflect(const Vector3& v1, const Vector3& v2)
{
	return v1 - (2.f * Vector3::Dot(v1, v2) * v2);
}

Vector3 Vector3::Refract(const Vector3& incident, const Vector3& normal, const float eta)
{
	const float cos_i{ std::clamp(Vector3::Dot(-incident, normal), -1.f, 1.f) };
	const float sin2_t{ eta * eta * (1.f - cos_i * cos_i) };
	if (sin2_t > 1.f) return {}; // total internal reflection
	const float cos_t{ std::sqrt(1.f - sin2_t) };
	return incident * eta + normal * (eta * cos_i - cos_t);
}

Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
{
	return {
		std::max(v1.x,v2.x),
		std::max(v1.y,v2.y),
		std::max(v1.z,v2.z) };
}

Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
{
	return {
		std::min(v1.x,v2.x),
		std::min(v1.y,v2.y),
		std::min(v1.z,v2.z) };
}

#pragma region Operator Overloads
Vector3 Vector3::operator*(const float scale) const
{
	return { x * scale, y * scale, z * scale };
}

Vector3 Vector3::operator/(const float scale) const
{
	assert(scale != 0.f && "Division by zero");
	return { x / scale, y / scale, z / scale };
}

Vector3 Vector3::operator+(const Vector3& v) const
{
	return { x + v.x, y + v.y, z + v.z };
}

Vector3 Vector3::operator-(const Vector3& v) const
{
	return { x - v.x, y - v.y, z - v.z };
}

Vector3 Vector3::operator-() const
{
	return { -x ,-y,-z };
}

Vector3& Vector3::operator*=(const float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}

Vector3& Vector3::operator/=(const float scale)
{
	assert(scale != 0.f && "Division by zero");
	x /= scale;
	y /= scale;
	z /= scale;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3& Vector3::operator+=(const Vector3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

float& Vector3::operator[](const uint8_t index)
{
	assert(index <= 2);
	if (index == 0) return x;
	if (index == 1) return y;
	return z;
}

float Vector3::operator[](const uint8_t index) const
{
	assert(index <= 2);
	if (index == 0) return x;
	if (index == 1) return y;
	return z;
}

bool Vector3::operator==(const Vector3& v) const
{
	auto eq = [](const float x, const float y)
		{
			const float diff{ std::fabs(x - y) };
			const float scale{ std::max({ 1.0f, std::fabs(x), std::fabs(y) }) };
			return diff <= std::numeric_limits<float>::epsilon() * scale;
		};
	return eq(x, v.x) && eq(y, v.y) && eq(z, v.z);
}

bool Vector3::operator!=(const Vector3& v) const
{
	return !(*this == v);
}
#pragma endregion
