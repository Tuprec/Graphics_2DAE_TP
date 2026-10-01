//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <software_path_tracer.h>
#include <scenes.h>
#include <intersections.h>
#include <numbers>

using namespace gfx;

// =============================================================================
// Construction / Destruction
// =============================================================================
SoftwarePathTracer::SoftwarePathTracer(Context* const context)
	: Renderer(context)
{
}

SoftwarePathTracer::~SoftwarePathTracer() = default;

// =============================================================================
// Public Functions
// =============================================================================
void SoftwarePathTracer::Render()
{
	assert(context_ && "Context not available!");

	const SurfaceInfo& surface_info = context_->surface_info;
	const float aspect_ratio{ surface_info.width / static_cast<float> (surface_info.height) };


	////Week1
	////EX2
	//const Sphere test_sphere{ Vector3{ 0.f,0.f,100.f },50.f };
	//const float fov{ 1 };
	////EX3 
	//const Plane test_plane_inf{ Vector3{ 0.f,-50.f,0.f } ,Vector3{ 0.f,1.f,0.f } };
	//const Plane test_plane_finite{ Vector3{ 0.f,-50.f,0.f } ,Vector3{ 0.f,1.f,0.f }, true,Vector2{100.f,100.f} };
	//const float fov{ 1 };

	////EX4
	const float fov_in_radiants{ context_->scene_manager->GetActiveScene()->camera.GetFovAngle() / 180.f * static_cast<float>(std::numbers::pi) };
	const float fov{ tanf(fov_in_radiants / 2.f) };
	const Scene* pScene{ context_->scene_manager->GetActiveScene() };


	for (uint32_t py = 0; py < surface_info.height; ++py)
	{
		for (uint32_t px = 0; px < surface_info.width; ++px)
		{
			////Week1
			//W1Ex1(surface_info, px, py, aspect_ratio, fov);
			//W1Ex2(surface_info, px, py, aspect_ratio, fov, test_sphere);
			//W1Ex3(surface_info, px, py, aspect_ratio, fov, test_plane_inf);
			//W1Ex3(surface_info, px, py, aspect_ratio, fov, test_plane_finite);
			//W1Ex4(surface_info, pScene, px, py, aspect_ratio, fov);

			//Week2
			W2Ex4(surface_info, pScene, px, py, aspect_ratio, fov);


		}
	}
}

bool gfx::SoftwarePathTracer::SceneClosestHitTest(const Scene* scene, const Ray& ray, RayHitRecord& closest_hit, bool ignore_hit_record) const
{
	bool has_hit{};
	for (const gfx::SceneObject& object : scene->objects)
	{
		const Primitive* prim{ scene->primitives_factory.Get(object.primitive_index) };
		const PrimitiveType type{ prim->type };
		bool hit{};
		if (type == PrimitiveType::kPlane)
		{
			const Plane plane{ *prim->CloneAs<Plane>() };
			hit = HitTestPlane(plane, ray, closest_hit, ignore_hit_record);

		}
		else if (type == PrimitiveType::kSphere)
		{
			const Sphere sphere{ *prim->CloneAs<Sphere>() };
			hit = HitTestSphere(sphere, ray, closest_hit, ignore_hit_record);
		}

		if (hit)
		{
			closest_hit.object_index = object.primitive_index;
			has_hit = true;
		}
	}
	return has_hit;
}

ShadingInput gfx::SoftwarePathTracer::ConstructShadingInput(const Scene* scene, const RayHitRecord& hit) const
{
	const Vector3 p{ hit.ray.origin + hit.t * hit.ray.direction };
	const PrimitiveType type{ scene->primitives_factory.Get(hit.object_index)->type };

	ShadingInput input{};
	input.view_direction = hit.ray.direction - hit.ray.origin;
	input.view_direction.Normalize();

	input.world_position = p;
	if (type == PrimitiveType::kPlane)
	{
		input.world_normal = scene->primitives_factory.Get(hit.object_index)->CloneAs<Plane>()->normal.Normalized();
	}
	else if (type == PrimitiveType::kSphere)
	{
		input.world_normal = (input.world_position - scene->primitives_factory.Get(hit.object_index)->CloneAs<Sphere>()->origin).Normalized();;
	}
	return input;
}

void gfx::SoftwarePathTracer::W1Ex1(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov) const
{
	Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
	ray_direction.x *= aspect_ratio * fov;
	ray_direction.y *= fov;
	ray_direction.Normalize();

	// Convert gradient value to color.
	ColorRgba final_color{ ray_direction.x, ray_direction.y, ray_direction.z };
	final_color.MaxToOne();

	// Write to surface	
	surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
		surface_info.pixel_format_details, nullptr,
		static_cast<uint8_t>(final_color.r * 255),
		static_cast<uint8_t>(final_color.g * 255),
		static_cast<uint8_t>(final_color.b * 255));
}

void gfx::SoftwarePathTracer::W1Ex2(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov, const Sphere& test_sphere, const Vector3& camera_origin) const
{
	Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
	ray_direction.x *= aspect_ratio * fov;
	ray_direction.y *= fov;
	ray_direction.Normalize();

	RayHitRecord closest_hit_record{};
	Ray view_ray{ camera_origin,ray_direction };
	bool did_hit{ HitTestSphere(test_sphere,view_ray,closest_hit_record) };
	ShadingInput shading_input{};

	if (did_hit && closest_hit_record.t > view_ray.min && closest_hit_record.t < view_ray.max)
	{
		return;
	}
	const Vector3 p{ view_ray.origin + closest_hit_record.t * view_ray.direction };
	shading_input.world_position = p;
	shading_input.world_normal = (shading_input.world_position - test_sphere.origin).Normalized();

	const VisualizationMode& visMode{ context_->debug_params.visualization_mode };
	ColorRgba final_color{};
	if (visMode == VisualizationMode::kDepth)
	{
		const float max_depth{ 100.f };
		const float scaled_t{ 1.f - std::clamp(closest_hit_record.t / max_depth,0.f,1.f) };
		final_color = ColorRgba{ scaled_t, scaled_t, scaled_t };
	}
	else if (visMode == VisualizationMode::kNone)
	{
		final_color = ColorRgba{ 1.f,0.f,0.f };
	}
	else if (visMode == VisualizationMode::kNormals)
	{
		const Vector3& n{ shading_input.world_normal };
		final_color = ColorRgba{ (n.x + 1.f) * 0.5f, (n.y + 1.f) * 0.5f, (n.z + 1.f) * 0.5f };
	}
	final_color.MaxToOne();

	// Write to surface	
	surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
		surface_info.pixel_format_details, nullptr,
		static_cast<uint8_t>(final_color.r * 255),
		static_cast<uint8_t>(final_color.g * 255),
		static_cast<uint8_t>(final_color.b * 255));
}

void gfx::SoftwarePathTracer::W1Ex3(const SurfaceInfo& surface_info, uint32_t px, uint32_t py, float aspect_ratio, float fov, const Plane& test_plane, const Vector3& camera_origin) const
{
	Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
	ray_direction.x *= aspect_ratio * fov;
	ray_direction.y *= fov;
	ray_direction.Normalize();

	RayHitRecord closest_hit_record{};
	Ray view_ray{ camera_origin,ray_direction };
	bool did_hit{ HitTestPlane(test_plane,view_ray,closest_hit_record) };
	ShadingInput shading_input{};
	if (!did_hit)
	{
		return;
	}
	const Vector3 p{ view_ray.origin + closest_hit_record.t * view_ray.direction };
	shading_input.world_position = p;
	shading_input.world_normal = test_plane.normal.Normalized();

	const VisualizationMode& visMode{ context_->debug_params.visualization_mode };
	ColorRgba final_color{};
	if (visMode == VisualizationMode::kDepth)
	{
		const float max_depth{ 100.f };
		const float scaled_t{ 1.f - std::clamp(closest_hit_record.t / max_depth,0.f,1.f) };
		final_color = ColorRgba{ scaled_t, scaled_t, scaled_t };
	}
	else if (visMode == VisualizationMode::kNone)
	{
		final_color = ColorRgba{ 1.f,0.f,0.f };
	}
	else if (visMode == VisualizationMode::kNormals)
	{
		const Vector3& n{ shading_input.world_normal };
		final_color = ColorRgba{ (n.x + 1) * 0.5f,(n.y + 1) * 0.5f,(n.z + 1) * 0.5f };
	}
	final_color.MaxToOne();

	// Write to surface	
	surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
		surface_info.pixel_format_details, nullptr,
		static_cast<uint8_t>(final_color.r * 255),
		static_cast<uint8_t>(final_color.g * 255),
		static_cast<uint8_t>(final_color.b * 255));
}

void gfx::SoftwarePathTracer::W1Ex4(const SurfaceInfo& surface_info, const Scene* pScene, uint32_t px, uint32_t py, float aspect_ratio, float fov)
{
	RayHitRecord closest_hit_record{};
	ShadingInput shading_input{};
	Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
	ray_direction.x *= aspect_ratio * fov;
	ray_direction.y *= fov;
	ray_direction.Normalize();

	const Ray view_ray{ pScene->camera.GetPosition(), ray_direction };

	bool did_hit{ SceneClosestHitTest(pScene,view_ray,closest_hit_record) };

	ColorRgba final_color{};
	if (did_hit)
	{
		shading_input = ConstructShadingInput(pScene, closest_hit_record);

		const VisualizationMode& visual_mode{ context_->debug_params.visualization_mode };
		if (visual_mode == VisualizationMode::kDepth)
		{
			const float max_depth{ 100.f };
			const float scaled_t{ 1.f - std::clamp(closest_hit_record.t / max_depth,0.f,1.f) };
			final_color = ColorRgba{ scaled_t, scaled_t, scaled_t };
		}
		else if (visual_mode == VisualizationMode::kNone)
		{
			const uint32_t idx{ closest_hit_record.object_index };
			final_color = {
				static_cast<float>(idx & 1),
				static_cast<float>((idx >> 1) & 1),
				static_cast<float>((idx >> 2) & 1)
			};
		}
		else if (visual_mode == VisualizationMode::kNormals)
		{
			const Vector3& n{ shading_input.world_normal };
			final_color = ColorRgba{ (n.x + 1.f) * 0.5f, (n.y + 1.f) * 0.5f, (n.z + 1.f) * 0.5f };
		}
		final_color.MaxToOne();
	}
	else
	{
		final_color = pScene->background_color;
	}
	// Write to surface	
	surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
		surface_info.pixel_format_details, nullptr,
		static_cast<uint8_t>(final_color.r * 255),
		static_cast<uint8_t>(final_color.g * 255),
		static_cast<uint8_t>(final_color.b * 255));
}

void gfx::SoftwarePathTracer::W2Ex1(const SurfaceInfo& surface_info, const Scene* pScene, uint32_t px, uint32_t py, float aspect_ratio, float fov)
{
	RayHitRecord closest_hit_record{};
	ShadingInput shading_input{};
	Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
	ray_direction.x *= aspect_ratio * fov;
	ray_direction.y *= fov;
	ray_direction.Normalize();

	const Ray view_ray{ pScene->camera.GetPosition(), ray_direction };

	bool did_hit{ SceneClosestHitTest(pScene,view_ray,closest_hit_record) };

	ColorRgba final_color{};
	if (did_hit)
	{
		shading_input = ConstructShadingInput(pScene, closest_hit_record);

		const VisualizationMode& visual_mode{ context_->debug_params.visualization_mode };
		if (visual_mode == VisualizationMode::kDepth)
		{
			const float max_depth{ 100.f };
			const float scaled_t{ 1.f - std::clamp(closest_hit_record.t / max_depth,0.f,1.f) };
			final_color = ColorRgba{ scaled_t, scaled_t, scaled_t };
		}
		else if (visual_mode == VisualizationMode::kNone)
		{
			const uint32_t idx{ closest_hit_record.object_index };
			final_color = {
				static_cast<float>(idx & 1),
				static_cast<float>((idx >> 1) & 1),
				static_cast<float>((idx >> 2) & 1)
			};
		}
		else if (visual_mode == VisualizationMode::kNormals)
		{
			const Vector3& n{ shading_input.world_normal };
			final_color = ColorRgba{ (n.x + 1.f) * 0.5f, (n.y + 1.f) * 0.5f, (n.z + 1.f) * 0.5f };
		}
		final_color.MaxToOne();
	}
	else
	{
		final_color = pScene->background_color;
	}
	// Write to surface	
	surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
		surface_info.pixel_format_details, nullptr,
		static_cast<uint8_t>(final_color.r * 255),
		static_cast<uint8_t>(final_color.g * 255),
		static_cast<uint8_t>(final_color.b * 255));
}
