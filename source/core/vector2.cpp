//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <vector2.h>
#include <vector3.h>
#include <vector4.h>
using namespace gfx;

//--- Standard Includes ---
#include <algorithm>
#include <cassert>
#include <cmath>

Vector2::Vector2(const float _x, const float _y) : x(_x), y(_y) {}

Vector2::Vector2(const Vector3& v) : x(v.x), y(v.y) {}

Vector2::Vector2(const Vector4& v) : x(v.x), y(v.y) {}

float Vector2::Magnitude() const
{
	return std::sqrt(x * x + y * y);
}

float Vector2::SqrMagnitude() const
{
	return x * x + y * y;
}

float Vector2::Normalize()
{
	const float m{ Magnitude() };
	if (m > 0.f) { x /= m; y /= m; }
	return m;
}

Vector2 Vector2::Normalized() const
{
	const float m{ Magnitude() };
	if (m > 0.f) return { x / m, y / m };
	return {};
}

float Vector2::Dot(const Vector2& v1, const Vector2& v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

float Vector2::Cross(const Vector2& v1, const Vector2& v2)
{
	return v1.x * v2.y - v1.y * v2.x;
}

float Vector2::Cross(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.y - v1.y * v2.x;
}

float Vector2::Cross(const Vector4& v1, const Vector4& v2)
{
	return v1.x * v2.y - v1.y * v2.x;
}

Vector2 Vector2::Reflect(const Vector2& v1, const Vector2& v2)
{
	return v1 - (2.f * Vector2::Dot(v1, v2) * v2);
}

Vector2 Vector2::Max(const Vector2& v1, const Vector2& v2)
{
	return {
		std::max(v1.x,v2.x),
		std::max(v1.y,v2.y) };
}

Vector2 Vector2::Min(const Vector2& v1, const Vector2& v2)
{
	return {
		std::min(v1.x,v2.x),
		std::min(v1.y,v2.y) };
}

Vector2 Vector2::Clamp(const Vector2& v, const float min, const float max)
{
	return {
		std::clamp(v.x, min, max),
		std::clamp(v.y, min, max) };
}

#pragma region Operator Overloads
Vector2 Vector2::operator*(const float scale) const
{
	return { x * scale, y * scale };
}

Vector2 Vector2::operator/(const float scale) const
{
	assert(scale != 0.f && "Division by zero");
	return { x / scale, y / scale };
}

Vector2 Vector2::operator+(const Vector2& v) const
{
	return { x + v.x, y + v.y };
}

Vector2 Vector2::operator-(const Vector2& v) const
{
	return { x - v.x, y - v.y };
}

Vector2 Vector2::operator-() const
{
	return { -x ,-y };
}

Vector2& Vector2::operator*=(const float scale)
{
	x *= scale;
	y *= scale;
	return *this;
}

Vector2& Vector2::operator/=(const float scale)
{
	assert(scale != 0.f && "Division by zero");
	x /= scale;
	y /= scale;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vector2& Vector2::operator+=(const Vector2& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

float& Vector2::operator[](const uint8_t index)
{
	assert(index <= 1);
	if (index == 0) return x;
	return y;
}

float Vector2::operator[](const uint8_t index) const
{
	assert(index <= 1);
	if (index == 0) return x;
	return y;
}

bool Vector2::operator==(const Vector2& v) const
{
	auto eq = [](const float x, const float y)
		{
			const float diff{ std::fabs(x - y) };
			const float scale{ std::max({ 1.0f, std::fabs(x), std::fabs(y) }) };
			return diff <= std::numeric_limits<float>::epsilon() * scale;
		};
	return eq(x, v.x) && eq(y, v.y);
}

bool Vector2::operator!=(const Vector2& v) const
{
	return !(*this == v);
}
#pragma endregion
