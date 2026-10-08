//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <matrix.h>
using namespace gfx;

//--- Standard Includes ---
#include <cassert>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

//--- Framework Includes ---
#include <primitives.h>

Matrix::Matrix(const Vector3& x_axis, const Vector3& y_axis, const Vector3& z_axis, const Vector3& t) :
	Matrix({ x_axis.x, x_axis.y, x_axis.z, 0 }, { y_axis.x, y_axis.y, y_axis.z, 0 },
		{ z_axis.x, z_axis.y, z_axis.z, 0 }, { t.x, t.y, t.z, 1 })
{
}

Matrix::Matrix(const Vector4& x_axis, const Vector4& y_axis, const Vector4& z_axis, const Vector4& t)
{
	data[0] = x_axis;
	data[1] = y_axis;
	data[2] = z_axis;
	data[3] = t;
}

Matrix::Matrix(const Matrix& m)
{
	data[0] = m[0];
	data[1] = m[1];
	data[2] = m[2];
	data[3] = m[3];
}

Matrix& Matrix::operator=(const Matrix& m)
{
	if (this == &m) return *this;

	data[0] = m[0];
	data[1] = m[1];
	data[2] = m[2];
	data[3] = m[3];
	InvalidateCache();
	return *this;
}

Matrix::Matrix(Matrix&& m) noexcept
{
	data[0] = m.data[0];
	data[1] = m.data[1];
	data[2] = m.data[2];
	data[3] = m.data[3];
}

Matrix& Matrix::operator=(Matrix&& m) noexcept
{
	if (this == &m) return *this;

	data[0] = m.data[0];
	data[1] = m.data[1];
	data[2] = m.data[2];
	data[3] = m.data[3];

	InvalidateCache();
	return *this;
}

Vector3 Matrix::TransformVector(const Vector3& v) const
{
	return TransformVector(v[0], v[1], v[2]);
}

Vector3 Matrix::TransformVector(const float x, const float y, const float z) const
{
	return Vector3{
		data[0].x * x + data[1].x * y + data[2].x * z,
		data[0].y * x + data[1].y * y + data[2].y * z,
		data[0].z * x + data[1].z * y + data[2].z * z };
}

Vector4 Matrix::TransformVector(const Vector4& v) const
{
	return TransformVector(v[0], v[1], v[2], v[3]);
}

Vector4 Matrix::TransformVector(const float x, const float y, const float z, const float w) const
{
	(void)w;
	return Vector4{
		data[0].x * x + data[1].x * y + data[2].x * z,
		data[0].y * x + data[1].y * y + data[2].y * z,
		data[0].z * x + data[1].z * y + data[2].z * z,
		0.f };
}

Vector3 Matrix::TransformPoint(const Vector3& p) const
{
	return TransformPoint(p[0], p[1], p[2]);
}

Vector3 Matrix::TransformPoint(const float x, const float y, const float z) const
{
	return Vector3{
		data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
		data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
		data[0].z * x + data[1].z * y + data[2].z * z + data[3].z };
}

Vector4 Matrix::TransformPoint(const Vector4& p) const
{
	return TransformPoint(p.x, p.y, p.z, p.w);
}

Vector4 Matrix::TransformPoint(const float x, const float y, const float z, const float w) const
{
	return Vector4{
		data[0].x * x + data[1].x * y + data[2].x * z + data[3].x * w,
		data[0].y * x + data[1].y * y + data[2].y * z + data[3].y * w,
		data[0].z * x + data[1].z * y + data[2].z * z + data[3].z * w,
		data[0].w * x + data[1].w * y + data[2].w * z + data[3].w * w };
}

Vector3 Matrix::TransformNormal(const Vector3& n, const bool is_non_uniform) const
{
	if (!is_non_uniform)
		return TransformVector(n).Normalized();

	const Matrix& inverse = GetInverse();
	const Matrix transpose_inverse = Matrix::Transpose(inverse);
	return transpose_inverse.TransformVector(n).Normalized();
}

Ray Matrix::TransformRay(const Ray& ray) const
{
	const Matrix& inverse = GetInverse();
	Ray transformed_ray{};
	transformed_ray.origin = inverse.TransformPoint(ray.origin);
	transformed_ray.direction = inverse.TransformVector(ray.direction);
	transformed_ray.min = ray.min;
	transformed_ray.max = ray.max;
	return transformed_ray;
}

const Matrix& Matrix::Inverse()
{
	// REMOVED as this does not work properly with projection matrices... Investigate!
	//-------------------------------------------------------------------------
	//Optimized Inverse as explained in FGED1 - used widely in other libraries too.
	/*const Vector3& a = data[0];
	const Vector3& b = data[1];
	const Vector3& c = data[2];
	const Vector3& d = data[3];

	const float x = data[0][3];
	const float y = data[1][3];
	const float z = data[2][3];
	const float w = data[3][3];

	Vector3 s = Vector3::Cross(a, b);
	Vector3 t = Vector3::Cross(c, d);
	Vector3 u = a * y - b * x;
	Vector3 v = c * w - d * z;

	const float det = Vector3::Dot(s, v) + Vector3::Dot(t, u);
	const bool is_zero = std::abs(det - 0.f) < std::numeric_limits<float>::epsilon();
	assert(!is_zero && "ERROR: determinant is 0, there is no INVERSE!"); (void)is_zero;
	const float inv_det = 1.f / det;

	s *= inv_det; t *= inv_det; u *= inv_det; v *= inv_det;

	const Vector3 r0 = Vector3::Cross(b, v) + t * y;
	const Vector3 r1 = Vector3::Cross(v, a) - t * x;
	const Vector3 r2 = Vector3::Cross(d, u) + s * w;

	data[0] = Vector4{ r0.x, r1.x, r2.x, 0.f };
	data[1] = Vector4{ r0.y, r1.y, r2.y, 0.f };
	data[2] = Vector4{ r0.z, r1.z, r2.z, 0.f };
	data[3] = { -Vector3::Dot(r0, d), -Vector3::Dot(r1, d), -Vector3::Dot(r2, d), Vector3::Dot(c, s) + Vector3::Dot(d, u) };
	InvalidateCache();
	return *this;*/
	//-------------------------------------------------------------------------

	// Create augmented matrix [A|I] where A is this matrix and I is identity.
	float augmented[4][8];

	// Fill left side with current matrix.
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t j = 0; j < 4; j++)
		{
			augmented[i][j] = data[i][j];
		}
	}

	// Fill right side with identity matrix.
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t j = 4; j < 8; j++)
		{
			augmented[i][j] = (i == (j - 4)) ? 1.0f : 0.0f;
		}
	}

	// Gaussian elimination with partial pivoting.
	for (uint8_t col = 0; col < 4; col++)
	{
		// Find the row with the largest absolute value in current column (partial pivoting).
		int pivot_row = col;
		float max_val = std::abs(augmented[col][col]);

		for (uint8_t row = col + 1; row < 4; row++)
		{
			if (std::abs(augmented[row][col]) > max_val)
			{
				max_val = std::abs(augmented[row][col]);
				pivot_row = row;
			}
		}

		// Check for singular matrix.
		if (max_val < std::numeric_limits<float>::epsilon())
		{
			// Matrix is singular, return identity.
			*this = CreateIdentity();
			InvalidateCache();
			return *this;
		}

		// Swap rows if needed.
		if (std::cmp_not_equal(pivot_row, col))
		{
			for (uint8_t j = 0; j < 8; j++)
			{
				std::swap(augmented[col][j], augmented[pivot_row][j]);
			}
		}

		// Scale pivot row to make diagonal element 1.
		const float pivot = augmented[col][col];
		for (uint8_t j = 0; j < 8; j++)
			augmented[col][j] /= pivot;

		// Eliminate other elements in this column.
		for (uint8_t row = 0; row < 4; row++)
		{
			if (row != col)
			{
				const float factor = augmented[row][col];
				for (uint8_t j = 0; j < 8; j++)
					augmented[row][j] -= factor * augmented[col][j];
			}
		}
	}

	// Extract inverse matrix from right side of augmented matrix.
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t j = 0; j < 4; j++)
			data[i][j] = augmented[i][j + 4];
	}

	InvalidateCache();
	return *this;
}

void Matrix::InvalidateCache() const
{
	cached_inverse.reset();
	inverse_computed.store(false, std::memory_order_release);
}

const Matrix& Matrix::Transpose()
{
	Matrix result = {};
	for (uint8_t r = 0; r < 4; ++r)
	{
		for (uint8_t c = 0; c < 4; ++c)
		{
			result[r][c] = data[c][r];
		}
	}

	data[0] = result[0];
	data[1] = result[1];
	data[2] = result[2];
	data[3] = result[3];
	InvalidateCache();
	return *this;
}

const Matrix& Matrix::GetInverse() const
{
	if (!inverse_computed.load(std::memory_order_acquire))
	{
		cached_inverse = std::make_unique<Matrix>(*this);
		const float det{ cached_inverse->Determinant() };
		// If matrix is not invertible, use identity instead!
		if (std::abs(det) < std::numeric_limits<float>::epsilon())
			*cached_inverse = CreateIdentity();
		else
			cached_inverse->Inverse();
		inverse_computed.store(true, std::memory_order_release);
	}
	return *cached_inverse;
}

float Matrix::Determinant() const
{
	const float a{ data[0][0] }, b{ data[0][1] }, c{ data[0][2] }, d{ data[0][3] };
	const float e{ data[1][0] }, f{ data[1][1] }, g{ data[1][2] }, h{ data[1][3] };
	const float i{ data[2][0] }, j{ data[2][1] }, k{ data[2][2] }, l{ data[2][3] };
	const float m{ data[3][0] }, n{ data[3][1] }, o{ data[3][2] }, p{ data[3][3] };

	return a * (f * (k * p - l * o) - g * (j * p - l * n) + h * (j * o - k * n)) -
		b * (e * (k * p - l * o) - g * (i * p - l * m) + h * (i * o - k * m)) +
		c * (e * (j * p - l * n) - f * (i * p - l * m) + h * (i * n - j * m)) -
		d * (e * (j * o - k * n) - f * (i * o - k * m) + g * (i * n - j * m));
}

Matrix Matrix::Transpose(const Matrix& m)
{
	Matrix out = m;
	out.Transpose();
	return out;
}

Matrix Matrix::Inverse(const Matrix& m)
{
	Matrix out = m;
	out.Inverse();
	return out;
}

Matrix Matrix::CreateLookAtLH(const Vector3& origin, const Vector3& forward, const Vector3& up)
{
	const Vector3 right{ Vector3::Cross(up,forward).Normalized() };
	const Vector3 postion{ -Vector3::Dot(origin,Vector3::UnitX()),-Vector3::Dot(origin,Vector3::UnitY()) ,-Vector3::Dot(origin,Vector3::UnitZ()) };
	return {
		right.ToVector4(),//{x,y,z,0}
		up.ToVector4(),//{x,y,z,0}
		forward.ToVector4(),//{x,y,z,0}
		postion.ToPoint4()//{x,y,z,1}
	};

}

Matrix Matrix::CreatePerspectiveFovLH(const float fov_y, const float aspect, const float zn, const float zf)
{
	//TODO
	assert(false && "Not Implemented");
	(void)fov_y; (void)aspect; (void)zn; (void)zf;
	return {};
}


Vector3 Matrix::GetAxisX() const
{
	return data[0];
}

Vector3 Matrix::GetAxisY() const
{
	return data[1];
}

Vector3 Matrix::GetAxisZ() const
{
	return data[2];
}

Vector3 Matrix::GetTranslation() const
{
	return data[3];
}

Vector3 Matrix::GetScale() const
{
	const Vector3 x_axis{ GetAxisX() };
	const Vector3 y_axis{ GetAxisY() };
	const Vector3 z_axis{ GetAxisZ() };

	return Vector3{
		x_axis.Magnitude(),
		y_axis.Magnitude(),
		z_axis.Magnitude()
	};
}

void Matrix::Decompose(Vector3& scale, Vector3& rotation, Vector3& translation) const
{
	// Based on standard TRS (Translation-Rotation-Scale).
	translation = GetTranslation();
	scale = GetScale();

	const Vector3 x_axis{ GetAxisX() / scale.x };
	const Vector3 y_axis{ GetAxisY() / scale.y };
	const Vector3 z_axis{ GetAxisZ() / scale.z };

	rotation.x = std::atan2(y_axis.z, z_axis.z);
	rotation.y = std::atan2(-x_axis.z, std::sqrt(y_axis.z * y_axis.z + z_axis.z * z_axis.z));
	rotation.z = std::atan2(x_axis.y, x_axis.x);
}

Matrix Matrix::CreateIdentity()
{
	return Matrix{};
}

Matrix Matrix::CreateTranslation(const float x, const float y, const float z)
{
	return CreateTranslation({ x, y, z });
}

Matrix Matrix::CreateTranslation(const Vector3& t)
{
	return { Vector3::UnitX(), Vector3::UnitY(), Vector3::UnitZ(), t };
}

Matrix Matrix::CreateRotationX(const float pitch, const bool in_degrees)
{
	const float angle = in_degrees ? ToRadians(pitch) : pitch;
	return {
		{1, 0, 0, 0},
		{0, std::cos(angle), -std::sin(angle), 0},
		{0, std::sin(angle), std::cos(angle), 0},
		{0, 0, 0, 1} };
}

Matrix Matrix::CreateRotationY(const float yaw, const bool in_degrees)
{
	const float angle = in_degrees ? ToRadians(yaw) : yaw;
	return {
		{std::cos(angle), 0, std::sin(angle), 0},
		{0, 1, 0, 0},
		{-std::sin(angle), 0, std::cos(angle), 0},
		{0, 0, 0, 1} };
}

Matrix Matrix::CreateRotationZ(const float roll, const bool in_degrees)
{
	const float angle = in_degrees ? ToRadians(roll) : roll;
	return {
		{std::cos(angle), -std::sin(angle), 0, 0},
		{std::sin(angle), std::cos(angle), 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1} };
}

Matrix Matrix::CreateRotation(const Vector3& r, const bool in_degrees)
{
	return CreateRotationX(r[0], in_degrees) * CreateRotationY(r[1], in_degrees) * CreateRotationZ(r[2], in_degrees);
}

Matrix Matrix::CreateRotation(float pitch, float yaw, float roll, const bool in_degrees)
{
	return CreateRotation({ pitch, yaw, roll }, in_degrees);
}

Matrix Matrix::CreateRotationAxis(const Vector3& axis, const float angle, const bool in_degrees)
{
	const float rad_angle{ in_degrees ? ToRadians(angle) : angle };
	const Vector3 normalized_axis{ axis.Normalized() };
	const float cos_angle{ std::cos(rad_angle) };
	const float sin_angle{ std::sin(rad_angle) };
	const float one_minus_cos{ 1.0f - cos_angle };

	const float x{ normalized_axis.x };
	const float y{ normalized_axis.y };
	const float z{ normalized_axis.z };

	return Matrix{
		{cos_angle + x * x * one_minus_cos, x * y * one_minus_cos - z * sin_angle, x * z * one_minus_cos + y * sin_angle, 0},
		{y * x * one_minus_cos + z * sin_angle, cos_angle + y * y * one_minus_cos, y * z * one_minus_cos - x * sin_angle, 0},
		{z * x * one_minus_cos - y * sin_angle, z * y * one_minus_cos + x * sin_angle, cos_angle + z * z * one_minus_cos, 0},
		{0, 0, 0, 1} };
}

Matrix Matrix::CreateScale(float sx, float sy, float sz)
{
	return { {sx, 0, 0, 0}, {0, sy, 0, 0}, {0, 0, sz, 0}, {0, 0, 0, 1} };
}

Matrix Matrix::CreateScale(const Vector3& s)
{
	return CreateScale(s[0], s[1], s[2]);
}

float Matrix::ToRadians(const float degrees)
{
	return degrees * static_cast<float>(std::numbers::pi) / 180.f;
}

Vector4& Matrix::operator[](const uint8_t index)
{
	assert(index < 4);
	return data[index];
}

const Vector4& Matrix::operator[](const uint8_t index) const
{
	assert(index < 4);
	return data[index];
}

Matrix Matrix::operator*(const Matrix& m) const
{
	Matrix result = {};

	for (uint8_t r = 0; r < 4; ++r)
	{
		for (uint8_t c = 0; c < 4; ++c)
		{
			result[r][c] = data[r][0] * m[0][c] + data[r][1] * m[1][c] +
				data[r][2] * m[2][c] + data[r][3] * m[3][c];
		}
	}
	return result;
}

const Matrix& Matrix::operator*=(const Matrix& m)
{
	Matrix copy = *this;

	for (uint8_t r = 0; r < 4; ++r)
	{
		for (uint8_t c = 0; c < 4; ++c)
		{
			data[r][c] = copy[r][0] * m[0][c] + copy[r][1] * m[1][c] +
				copy[r][2] * m[2][c] + copy[r][3] * m[3][c];
		}
	}
	InvalidateCache();
	return *this;
}

bool Matrix::operator==(const Matrix& m) const
{
	auto eq = [](const float x, const float y)
		{
			const float diff = std::fabs(x - y);
			const float scale = std::max({ 1.0f, std::fabs(x), std::fabs(y) });
			return diff <= std::numeric_limits<float>::epsilon() * scale;
		};

	for (uint8_t i = 0; i < 4; ++i)
	{
		for (uint8_t j = 0; j < 4; ++j)
		{
			if (!eq(data[i][j], m.data[i][j]))
				return false;
		}
	}
	return true;
}

bool Matrix::operator!=(const Matrix& m) const
{
	return !(*this == m);
}