//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <vector4.h>
#include <vector3.h>
#include <vector2.h>
using namespace gfx;

//--- Standard Includes ---
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>

Vector4::Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
Vector4::Vector4(const Vector3& v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {}

Vector3& Vector4::AsVector3()
{
	static_assert(offsetof(Vector4, x) == offsetof(Vector3, x)
		&& offsetof(Vector4, y) == offsetof(Vector3, y)
		&& offsetof(Vector4, z) == offsetof(Vector3, z),
		"Layout mismatch between Vector4 and Vector3");
	return reinterpret_cast<Vector3&>(*this);
}

Vector2& Vector4::AsVector2()
{
	static_assert(offsetof(Vector4, x) == offsetof(Vector2, x)
		&& offsetof(Vector4, y) == offsetof(Vector2, y),
		"Layout mismatch between Vector4 and Vector2");
	return reinterpret_cast<Vector2&>(*this);
}

float Vector4::Magnitude() const
{
	return std::sqrt(x * x + y * y + z * z + w * w);
}

float Vector4::SqrMagnitude() const
{
	return x * x + y * y + z * z + w * w;
}

float Vector4::Normalize()
{
	const float m{ Magnitude() };
	if (m > 0.f) { x /= m; y /= m; z /= m; w /= m; }
	return m;
}

Vector4 Vector4::Normalized() const
{
	const float m{ Magnitude() };
	if (m > 0.f) return { x / m, y / m, z / m, w / m };
	return {};
}

float Vector4::Dot(const Vector4& v1, const Vector4& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector4 Vector4::operator*(const float scale) const
{
	return { x * scale, y * scale, z * scale, w * scale };
}

Vector4 Vector4::operator/(const float scale) const
{
	assert(scale != 0.f && "Division by zero");
	return { x / scale, y / scale, z / scale, w / scale };
}

Vector4 Vector4::operator+(const Vector4& v) const
{
	return { x + v.x, y + v.y, z + v.z, w + v.w };
}

Vector4 Vector4::operator-(const Vector4& v) const
{
	return { x - v.x, y - v.y, z - v.z, w - v.w };
}

Vector4 Vector4::operator-() const
{
	return { -x, -y, -z, -w };
}

Vector4& Vector4::operator*=(const float scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;
	return *this;
}

Vector4& Vector4::operator/=(const float scale)
{
	assert(scale != 0.f && "Division by zero");
	x /= scale;
	y /= scale;
	z /= scale;
	w /= scale;
	return *this;
}

Vector4& Vector4::operator+=(const Vector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

Vector4& Vector4::operator-=(const Vector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

float& Vector4::operator[](const uint8_t index)
{
	assert(index <= 3);
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	return w;
}

float Vector4::operator[](const uint8_t index) const
{
	assert(index <= 3);
	if (index == 0) return x;
	if (index == 1) return y;
	if (index == 2) return z;
	return w;
}

bool Vector4::operator==(const Vector4& v) const
{
	auto eq = [](const float x, const float y)
		{
			const float diff{ std::fabs(x - y) };
			const float scale{ std::max({ 1.0f, std::fabs(x), std::fabs(y) }) };
			return diff <= std::numeric_limits<float>::epsilon() * scale;
		};
	return eq(x, v.x) && eq(y, v.y) && eq(z, v.z) && eq(w, v.w);
}

bool Vector4::operator!=(const Vector4& v) const
{
	return !(*this == v);
}
