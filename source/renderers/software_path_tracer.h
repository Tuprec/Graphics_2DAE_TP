//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef SOFTWARE_PATH_TRACER_HEADER
#define SOFTWARE_PATH_TRACER_HEADER

//--- Standard Includes ---

//--- Framework Includes ---
#include <renderer.h>

namespace gfx
{
	//--- Software Path Tracer ---
	class SoftwarePathTracer final : public Renderer
	{
	public:
		//--- Construction / Destruction ---
		SoftwarePathTracer(Context* const context);
		~SoftwarePathTracer() override;

		SoftwarePathTracer(const SoftwarePathTracer&) = delete;
		SoftwarePathTracer& operator=(const SoftwarePathTracer&) = delete;
		SoftwarePathTracer(SoftwarePathTracer&&) = delete;
		SoftwarePathTracer& operator=(SoftwarePathTracer&&) = delete;

		//--- Public Functions ---
		void Render() override;
	private:
		bool SceneClosestHitTest(const Scene* scene,const Ray& ray, RayHitRecord& closest_hit, bool ignore_hit_record = false) const;
		ShadingInput ConstructShadingInput(const Scene* scene, const RayHitRecord& hit) const;

		//--- Excercise funtions ---
		void W1Ex1(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov) const;
		void W1Ex2(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov, const Sphere& test_sphere, const Vector3& camera_origin = { 0.f,0.f,0.f }) const;
		void W1Ex3(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov, const Plane& test_plane, const Vector3& camera_origin = { 0.f,0.f,0.f }) const;
		void W1Ex4(const SurfaceInfo& surface_info, const Scene* pScene,  uint32_t px, uint32_t py, float aspect_ratio, float fov);
		
		
		void W2Ex1(const SurfaceInfo& surface_info,  Scene* pScene,  uint32_t px, uint32_t py, float aspect_ratio, float fov);
		void W2Ex2(const SurfaceInfo& surface_info,  Scene* pScene,  uint32_t px, uint32_t py, float aspect_ratio, float fov);
	};
}
#endif //SOFTWARE_PATH_TRACER_HEADER
