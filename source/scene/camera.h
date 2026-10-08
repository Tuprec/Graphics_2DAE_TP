//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef CAMERA_HEADER
#define CAMERA_HEADER

//--- Standard Includes ---
#include <cassert>
#include <numbers>
#include <iostream>

//--- Framework Includes ---
#include <matrix.h>
#include <vector3.h>

namespace gfx
{
	class Camera final
	{
		//--- Data members ---
		Matrix view_{ Matrix::CreateIdentity() };
		Matrix projection_{ Matrix::CreateIdentity() };

		Vector4 position_{ Vector4(Vector3::Zero(), 1.f) };
		Vector4 right_{ Vector4::UnitX() };
		Vector4 up_{ Vector4::UnitY() };
		Vector4 forward_{ Vector4::UnitZ() };

		float fov_angle_{ 60.f };
		float near_plane_{ 0.1f };
		float far_plane_{ 1000.f };
		bool dirty_{ true };

	public:
		//--- Constructors & Destructor ---
		Camera(const Vector3& position = Vector3::Zero(), const float fov_angle = 60.f) :
			position_(position, 1.f), fov_angle_(fov_angle) {}
		~Camera() = default;
		Camera(const Camera&) = default;
		Camera& operator=(const Camera&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(Camera&&) = default;

		//--- Functions ---
		void MoveForward(const float delta)
		{ position_ += forward_ * delta; dirty_ = true; }
		void MoveRight(const float delta)
		{ position_ += right_ * delta; dirty_ = true; }
		void MoveUp(const float delta)
		{ position_ += Vector4::UnitY() * delta; dirty_ = true; }

		void Pitch(const float angle)
		{
			const Matrix rotation = Matrix::CreateRotationAxis(right_, angle);
			forward_ = rotation.TransformVector(forward_);
			up_ = rotation.TransformVector(up_);
			dirty_ = true;
		}
		void Yaw(const float angle)
		{
			const Matrix rotation = Matrix::CreateRotationY(angle);
			forward_ = rotation.TransformVector(forward_);
			right_ = rotation.TransformVector(right_);
			dirty_ = true;
		}

		void SetPosition(const Vector3& position) { position_ = Vector4(position, 1.f); dirty_ = true; }
		void SetFovAngle(const float fov_angle) { fov_angle_ = fov_angle; dirty_ = true; }

		bool IsDirty() const { return dirty_; }
		void ClearDirty() { dirty_ = false; }

		const Vector4& GetPosition() const { return position_; }
		const Vector4& GetForward() const { return forward_; }
		float GetFovAngle() const { return fov_angle_; }

		const Matrix& GetView()
		{
			const Vector3 right{ Vector3::Cross(Vector3::UnitY(),forward_) };//Vector3::UnitY() is the world up;
			const Vector3 local_up{Vector3::Cross(forward_,right)};
			view_ = Matrix::CreateLookAtLH(position_, forward_, local_up).Transpose();
			return view_;
		}
		const Matrix& GetProjection(const float fov_y, const float aspect_ratio)
		{
			//TODO: create projection matrix
			(void)fov_y; (void)aspect_ratio;
			(void)near_plane_; (void)far_plane_;
			return projection_;
		}
	};
}
#endif //CAMERA_HEADER