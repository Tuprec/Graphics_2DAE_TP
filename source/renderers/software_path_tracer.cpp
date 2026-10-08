//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <software_path_tracer.h>
#include <scenes.h>
#include <intersections.h>
#include <numbers>
#include <numeric>//added by mathias
#include <execution>
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

	const SurfaceInfo& surface_info = context_->surface_info; //=> this holds info of your screen (width and height are for now the important once)
	const float aspect_ratio{ surface_info.width / static_cast<float> (surface_info.height) }; //=> this is used to prevent skewing/stretching on none sqaure screens when used to multiply x of ray_direction

	const float fov_in_radiants{ context_->scene_manager->GetActiveScene()->camera.GetFovAngle() / 180.f * static_cast<float>(std::numbers::pi) }; // => calculating the fov angle from degres to radians because the camera.GetFovAngle() returns the angle in degrees
	const float fov{ tanf(fov_in_radiants / 2.f) };// => calculating FOV(Field Of View)
	Scene* pScene{ context_->scene_manager->GetActiveScene() };// => retreving the pointer of te active scene
	const Matrix& cam_inverse{ pScene->camera.GetView().GetInverse() };
	
	const auto per_pixel_fnc = [&](const uint32_t pixel_idx)
		{
			const uint32_t px{ pixel_idx % surface_info.width };
			const uint32_t py{ pixel_idx / surface_info.width };
			RayHitRecord closest_hit_record{}; 	//Keeps track of the clossest hit with an object on the screen for the current pixel
			ShadingInput shading_input{}; //Shading input for this pixel

			//Calculate the Normalized Device Coordinates (NDC) 
			Vector3 ray_direction{ 2 * (px + 0.5f) / surface_info.width - 1,1 - 2 * (py + 0.5f) / surface_info.height,1.f };
			//Adding the aspectRatio to avoid skewing and multiplying to x, and multiplying with FOV
			ray_direction.x *= aspect_ratio * fov;
			ray_direction.y *= fov;
			ray_direction.Normalize();

			//Transform to take camera orientation into account and bringing the direction to world space
			ray_direction = cam_inverse.TransformVector(ray_direction);

			const Ray view_ray{ pScene->camera.GetPosition(), ray_direction }; // the ray that is cast from the camera to the current pixel

			bool did_hit{ SceneClosestHitTest(pScene,view_ray,closest_hit_record) }; //Checking if there was a hit

			ColorRgba final_color{};//=>Final color of a pixel
			if (did_hit)
			{
				shading_input = ConstructShadingInput(pScene, closest_hit_record);//Calculate the shading input

				const VisualizationMode& visual_mode{ context_->debug_params.visualization_mode };
				if (visual_mode == VisualizationMode::kDepth)
				{
					const float max_depth{ 100.f };
					const float scaled_t{ 1.f - std::clamp(closest_hit_record.t / max_depth,0.f,1.f) };
					final_color = ColorRgba{ scaled_t, scaled_t, scaled_t };
				}
				else if (visual_mode == VisualizationMode::kNone || visual_mode == VisualizationMode::kAlbedo)
				{
					//Bit shift based onn the object index
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
				//no hit means the pixel gets the background color of the scene
				final_color = pScene->background_color;
			}
			// Write to surface	
			surface_info.pixel_buffer[px + (py * surface_info.width)] = SDL_MapRGB(
				surface_info.pixel_format_details, nullptr,
				static_cast<uint8_t>(final_color.r * 255),
				static_cast<uint8_t>(final_color.g * 255),
				static_cast<uint8_t>(final_color.b * 255));
		};

	std::vector<uint32_t> pixel_indices( surface_info.width * surface_info.height );
	std::iota(pixel_indices.begin(), pixel_indices.end(), 0);
	std::for_each(std::execution::par, pixel_indices.begin(), pixel_indices.end(), per_pixel_fnc);
}

bool gfx::SoftwarePathTracer::SceneClosestHitTest(const Scene* scene, const Ray& ray, RayHitRecord& closest_hit, bool ignore_hit_record) const
{
	bool has_hit{}; // => Final boolean to be returned
	//Looping over every object in the scene
	Ray transformed_ray{ ray };
	for (size_t idx{ 0 }; idx < scene->objects.size(); ++idx)
	{
		const gfx::SceneObject& object{ scene->objects[idx] };
		//Retreving the primitive data and upcasting to the parent class
		const Primitive* prim{ scene->primitives_factory.Get(object.primitive_index) };
		const PrimitiveType type{ prim->type };
		//Temp variable to see if the current pixel has a hit
		bool hit{};

		if (object.instance_transformation.has_value())
		{
			transformed_ray = object.instance_transformation->TransformRay(ray);
		}

		if (type == PrimitiveType::kPlane)
		{
			const Plane* plane{ static_cast<const Plane*>(prim) };
			hit = HitTestPlane(*plane, transformed_ray, closest_hit, ignore_hit_record);//test if the ray hits the object 

		}
		else if (type == PrimitiveType::kSphere)
		{
			const Sphere* sphere{ static_cast<const Sphere*>(prim) };
			hit = HitTestSphere(*sphere, transformed_ray, closest_hit, ignore_hit_record);//test if the ray hits the object 
		}
		else if (type == PrimitiveType::kTriangle)
		{
			const Triangle* triangle{ static_cast<const Triangle*>(prim) };
			hit = HitTestTriangle(*triangle, transformed_ray, closest_hit, ignore_hit_record);//test if the ray hits the object 
		}
		else if (type == PrimitiveType::kTriangleMesh)
		{
			const TriangleMesh* triangle_mesh{ static_cast<const TriangleMesh*>(prim) };
			for (size_t indece{ 0 }; indece < triangle_mesh->indices.size(); indece += 3)
			{
				Triangle triangle{};
				triangle.cull_mode = triangle_mesh->cull_mode;

				//Indixes of the vertex
				const uint32_t i0{ triangle_mesh->indices[indece] };
				const uint32_t i1{ triangle_mesh->indices[indece + 1] };
				const uint32_t i2{ triangle_mesh->indices[indece + 2] };


				//Setting the vertexes of the triangle
				triangle.v0 = triangle_mesh->vertices[i0].position;
				triangle.v1 = triangle_mesh->vertices[i1].position;
				triangle.v2 = triangle_mesh->vertices[i2].position;

				//Calculating the normal of the triangle
				triangle.normal = Vector3::Cross((triangle.v1 - triangle.v0), (triangle.v2 - triangle.v0)).Normalized();

				hit = HitTestTriangle(triangle, transformed_ray, closest_hit, ignore_hit_record);
				if (hit && !ignore_hit_record)
				{
					closest_hit.vertex_indices = { i0,i1,i2 };
					closest_hit.object_index = static_cast<uint32_t>(idx);
					has_hit = true;
				}
			}
		}

		//if the ray hits save the primitive_index could be a problem in the futher and it needs to be changed to an incremented value of this loop
		//Note: The index had to change because the instances uses the same primitive index
		if (hit)
		{
			if (!ignore_hit_record)
			{
				closest_hit.object_index = static_cast<uint32_t>(idx);
			}
			has_hit = true;//Setting the main return variable to true because there was a hit.
		}
	}
	return has_hit;
}

ShadingInput gfx::SoftwarePathTracer::ConstructShadingInput(const Scene* scene, const RayHitRecord& hit) const
{
	const Vector3 p{ hit.ray.origin + hit.t * hit.ray.direction };//local Point of impact(hit point)
	const gfx::SceneObject& object{ scene->objects.at(hit.object_index) }; //the hit object
	const PrimitiveType type{ scene->primitives_factory.Get(object.primitive_index)->type }; // Type of the hit primitive

	ShadingInput input{};
	//No clue about this but it should be explained in week 4 and gets inverted
	input.view_direction = (hit.ray.origin - p).Normalized();

	if (object.instance_transformation.has_value())
	{
		input.world_position = object.instance_transformation->TransformPoint(p);//Bringing local hitpoint to world space
	}
	else
	{
		input.world_position = p;//=> the local point of impact(hit point)
	}

	if (type == PrimitiveType::kPlane)
	{
		input.world_normal = static_cast<Plane*>(scene->primitives_factory.Get(object.primitive_index))->normal.Normalized(); //Normal of the plane that got hit
	}
	else if (type == PrimitiveType::kSphere)
	{
		//Note: Had to use the local space point to calculate the normal
		input.world_normal = (p - static_cast<Sphere*>(scene->primitives_factory.Get(object.primitive_index))->origin).Normalized();//normal between the hitpoint and the center of the sphere
	}
	else if (type == PrimitiveType::kTriangle)
	{
		input.world_normal = static_cast<Triangle*>(scene->primitives_factory.Get(object.primitive_index))->normal.Normalized(); //Normal of the triangle that got hit
	}
	else if (type == PrimitiveType::kTriangleMesh && hit.vertex_indices.has_value())
	{
		const TriangleMesh* mesh{ static_cast<const TriangleMesh*>(scene->primitives_factory.Get(object.primitive_index)) };

		//The normals of the vertices
		const Vector3 n0{ mesh->vertices[hit.vertex_indices->at(0)].normal.value() };
		const Vector3 n1{ mesh->vertices[hit.vertex_indices->at(1)].normal.value() };
		const Vector3 n2{ mesh->vertices[hit.vertex_indices->at(2)].normal.value() };

		Vector2 u_and_v = hit.barycentric_coordinates.value();
		const float w{ 1 - u_and_v.x - u_and_v.y }; // calc w = 1-u-v;

		input.world_normal = (n0 * w + n1 * u_and_v.x + n2 * u_and_v.y).Normalized(); // Smooth shading

		
		//input.world_normal = (n0).Normalized(); // This is the normal used by the screenshot for non smooth shading

		//Used to calculate the triangle normal 
		//const Vector3 v0 = mesh.vertices[hit.vertex_indices->at(0)].position;
		//const Vector3 v1 = mesh.vertices[hit.vertex_indices->at(1)].position;
		//const Vector3 v2 = mesh.vertices[hit.vertex_indices->at(2)].position;
		//input.world_normal = Vector3::Cross(v1 - v0, v2 - v0).Normalized(); //=> this is the normal of the triangle
	}

	if (object.instance_transformation.has_value())
	{
		input.world_normal = object.instance_transformation->TransformNormal(input.world_normal).Normalized();
	}

	return input;
}