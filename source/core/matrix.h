//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef MATRIX_HEADER
#define MATRIX_HEADER

//--- Framework Includes ---
#include <vector3.h>
#include <vector4.h>

//--- Standard Includes ---
#include <atomic>
#include <memory>

namespace gfx
{
	//--- Row-Major Matrix ---
	struct Ray;
	struct Matrix final
	{
		//--- Constructors & Destructors ---
		Matrix(
			const Vector3& x_axis,
			const Vector3& y_axis,
			const Vector3& z_axis,
			const Vector3& t);
		Matrix(
			const Vector4& x_axis,
			const Vector4& y_axis,
			const Vector4& z_axis,
			const Vector4& t);
		Matrix() = default;
		~Matrix() = default;
		Matrix(const Matrix&);
		Matrix& operator=(const Matrix&);
		Matrix(Matrix&&) noexcept;
		Matrix& operator=(Matrix&&) noexcept;

		//--- Functions ---
		[[nodiscard]] Vector3 TransformVector(const Vector3& v) const;
		[[nodiscard]] Vector3 TransformVector(const float x, const float y, const float z) const;
		[[nodiscard]] Vector4 TransformVector(const Vector4& v) const;
		[[nodiscard]] Vector4 TransformVector(const float x, const float y, const float z, const float w) const;
		[[nodiscard]] Vector3 TransformPoint(const Vector3& p) const;
		[[nodiscard]] Vector3 TransformPoint(const float x, const float y, const float z) const;
		[[nodiscard]] Vector4 TransformPoint(const Vector4& p) const;
		[[nodiscard]] Vector4 TransformPoint(const float x, const float y, const float z, const float w) const;
		[[nodiscard]] Vector3 TransformNormal(const Vector3& n, const bool is_non_uniform = false) const;
		[[nodiscard]] Ray TransformRay(const Ray& ray) const;

		const Matrix& Transpose();
		const Matrix& GetInverse() const;
		[[nodiscard]] float Determinant() const;

		[[nodiscard]] Vector3 GetAxisX() const;
		[[nodiscard]] Vector3 GetAxisY() const;
		[[nodiscard]] Vector3 GetAxisZ() const;
		[[nodiscard]] Vector3 GetTranslation() const;
		[[nodiscard]] Vector3 GetScale() const;
		void Decompose(Vector3& scale, Vector3& rotation, Vector3& translation) const;

		static Matrix CreateIdentity();
		static Matrix CreateTranslation(const float x, const float y, const float z);
		static Matrix CreateTranslation(const Vector3& t);
		static Matrix CreateRotationX(const float pitch, const bool in_degrees = false);
		static Matrix CreateRotationY(const float yaw, const bool in_degrees = false);
		static Matrix CreateRotationZ(const float roll, const bool in_degrees = false);
		static Matrix CreateRotation(const float pitch, const float yaw, const float roll, const bool in_degrees = false);
		static Matrix CreateRotation(const Vector3& r, const bool in_degrees = false);
		static Matrix CreateRotationAxis(const Vector3& axis, const float angle, const bool in_degrees = false);
		static Matrix CreateScale(const float sx, const float sy, const float sz);
		static Matrix CreateScale(const Vector3& s);
		static Matrix Transpose(const Matrix& m);
		static Matrix Inverse(const Matrix& m);

		/// <summary>
		/// This function calculates and returns an ONB matrix(orthonormal basis )
		/// </summary>
		/// <param name="origin">This is te origin not the position from te matrix</param>
		/// <param name="forward">This is the forward</param>
		/// <param name="up">This is the local up</param>
		/// <returns>A 4x4 matrix (The ONB matrix)</returns>
		static Matrix CreateLookAtLH(const Vector3& origin, const Vector3& forward, const Vector3& up);
		static Matrix CreatePerspectiveFovLH(const float fov_y, const float aspect, const float zn, const float zf);

		//--- Operators ---
		Vector4& operator[](const uint8_t index);
		const Vector4& operator[](const uint8_t index) const;
		Matrix operator*(const Matrix& m) const;
		const Matrix& operator*=(const Matrix& m);
		bool operator==(const Matrix& m) const;
		bool operator!=(const Matrix& m) const;

	private:
		Vector4 data[4]
		{
			{1,0,0,0}, //xAxis
			{0,1,0,0}, //yAxis
			{0,0,1,0}, //zAxis
			{0,0,0,1}  //T
		};

		mutable std::atomic<bool> inverse_computed{ false };
		mutable std::unique_ptr<Matrix> cached_inverse;

		const Matrix& Inverse();
		void InvalidateCache() const;
		static float ToRadians(const float degrees);
	};
}
#endif //MATRIX_HEADER