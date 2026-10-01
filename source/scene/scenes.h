//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef SCENES_HEADER
#define SCENES_HEADER

//--- Standard Includes ---
#include <numbers>

//--- Framework Includes ---
#include <scene_manager.h>
#include <asset_loader.h>

namespace gfx
{
	// =========================================================================
	// PATH TRACER SCENES
	// =========================================================================
	// -------------------------------------------------------------------------
	// BasicScene: 3 spheres on a floor plane with a single point light.
	// -------------------------------------------------------------------------
	struct BasicScene final : public Scene
	{
		void Setup() override
		{
			// Camera Setup
			camera.SetPosition({ 0, 3, -9.f });
			camera.SetFovAngle(45.f);

			// Light Setup
			lights_factory.Create<Light>(Light{
				.origin = {0.f, 5.f, -5.f},
				.color = ColorRgba::White(),
				.intensity = 25.f,
				.type = LightType::kPoint });

			// Material Setup
			const uint32_t mat_red { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.8f, 0.2f, 0.2f) }) };
			const uint32_t mat_green { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.2f, 0.8f, 0.2f) }) };
			const uint32_t mat_blue { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.2f, 0.2f, 0.8f) }) };
			const uint32_t mat_white { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.7f, 0.7f, 0.7f) }) };

			// Spheres setup
			const uint32_t s0 { primitives_factory.Create<Sphere>(Vector3(-3.f, 1.f, 0.f), 1.f) };
			const uint32_t s1 { primitives_factory.Create<Sphere>(Vector3(0.f, 1.f, 0.f), 1.f) };
			const uint32_t s2 { primitives_factory.Create<Sphere>(Vector3(3.f, 1.f, 0.f), 1.f) };
			objects.push_back({ .primitive_index = s0, .material_index = mat_red });
			objects.push_back({ .primitive_index = s1, .material_index = mat_green });
			objects.push_back({ .primitive_index = s2, .material_index = mat_blue });

			// Floor plane setup
			const uint32_t floor { primitives_factory.Create<Plane>(
				Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)) };
			objects.push_back({ .primitive_index = floor, .material_index = mat_white });

			// Bounded back plane
			const uint32_t back { primitives_factory.Create<Plane>(
				Vector3(0.f, 2.f, 3.f), Vector3(0.f, 0.f, -1.f), true, Vector2(5.f, 2.f)) };
			objects.push_back({ .primitive_index = back, .material_index = mat_white });
		}
	};

	// -------------------------------------------------------------------------
	// InstanceScene: 3x3 grid of spheres from a single primitive (instancing).
	// -------------------------------------------------------------------------
	struct InstanceScene final : public Scene
	{
		void Setup() override
		{
			// Camera Setup
			camera.SetPosition({ 0, 3.5f, -9.f });
			camera.SetFovAngle(45.f);

			// Light Setup
			lights_factory.Create<Light>(Light{
				.origin = {0.f, 5.f, -5.f},
				.color = ColorRgba::White(),
				.intensity = 25.f,
				.type = LightType::kPoint });

			// Material Setup
			const uint32_t mat_white { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.7f, 0.7f, 0.7f) }) };

			// One sphere primitive, instanced 9 times
			const uint32_t sphere { primitives_factory.Create<Sphere>(Vector3(0.f, 0.f, 0.f), 1.f) };
			for (int row = 0; row < 3; ++row)
			{
				for (int col = 0; col < 3; ++col)
				{
					const float x = (col - 1) * 2.5f;
					const float y = 1.f + row * 2.5f;
					objects.push_back({
						.primitive_index = sphere,
						.material_index = mat_white,
						.instance_transformation = Matrix::CreateTranslation(x, y, 0.f) });
				}
			}
		}
	};

	// -------------------------------------------------------------------------
	// TriangleScene: 3 rotating triangles with different cull modes.
	// -------------------------------------------------------------------------
	struct TriangleScene final : public Scene
	{
		float rotation_angle{ 0.f };
		uint32_t tri0{ 0 }, tri1{ 0 }, tri2{ 0 };
		bool rotating_triangles{ true };

		void Setup() override
		{
			// Camera Setup
			camera.SetPosition({ 0, 2, -9.f });
			camera.SetFovAngle(45.f);

			// Light Setup
			lights_factory.Create<Light>(Light{
				.origin = {0.f, 5.f, -5.f},
				.color = ColorRgba::White(),
				.intensity = 25.f,
				.type = LightType::kPoint });

			// Material Setup
			const uint32_t mat_red { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.8f, 0.2f, 0.2f) }) };
			const uint32_t mat_green { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.2f, 0.8f, 0.2f) }) };
			const uint32_t mat_blue { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.2f, 0.2f, 0.8f) }) };
			const uint32_t mat_white { materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.7f, 0.7f, 0.7f) }) };

			// 3 triangles centered at origin, each with a different cull mode
			tri0 = primitives_factory.Create<Triangle>(
				Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 2.f, 0.f), Vector3(1.f, 0.f, 0.f),
				CullMode::kBackFaceCulling);
			tri1 = primitives_factory.Create<Triangle>(
				Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 2.f, 0.f), Vector3(1.f, 0.f, 0.f),
				CullMode::kFrontFaceCulling);
			tri2 = primitives_factory.Create<Triangle>(
				Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 2.f, 0.f), Vector3(1.f, 0.f, 0.f),
				CullMode::kNoCulling);

			objects.push_back({ .primitive_index = tri0, .material_index = mat_red });
			objects.push_back({ .primitive_index = tri1, .material_index = mat_green });
			objects.push_back({ .primitive_index = tri2, .material_index = mat_blue });

			// Floor plane
			const uint32_t floor{ primitives_factory.Create<Plane>(
				Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)) };
			objects.push_back({ .primitive_index = floor, .material_index = mat_white });
		}

		bool Update(const double delta_time) override
		{
			if (rotating_triangles)
			{
				constexpr float rotation_speed{ 75.0f };
				rotation_angle += static_cast<float>(delta_time * rotation_speed);
			}
			const Matrix rot{ Matrix::CreateRotationY(rotation_angle, true) };

			objects[0].instance_transformation = rot * Matrix::CreateTranslation(-3.f, 0.f, 0.f);
			objects[1].instance_transformation = rot;
			objects[2].instance_transformation = rot * Matrix::CreateTranslation(3.f, 0.f, 0.f);
			return true;
		}
	};

	// -------------------------------------------------------------------------
	// BunnyScene: Cornell box with a bunny mesh.
	// -------------------------------------------------------------------------
	struct BunnyScene final : public Scene
	{
		float light_time{ 0.f };
		uint32_t light_index{ 0 };
		bool rotating_lights{ true };

		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0, 3, -9.f });
			camera.SetFovAngle(45.f);

			// Light setup
			light_index =lights_factory.Create<Light>(Light{
				.origin = {0.f, 4.f, 5.f},
				.color = ColorRgba::White(),
				.intensity = 50.f,
				.type = LightType::kPoint });

			// Material setup
			const uint32_t mat_white{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{.diffuse_color = ColorRgba(0.7f, 0.7f, 0.7f) }) };
			const uint32_t mat_red{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.75f, 0.15f, 0.15f) }) };
			const uint32_t mat_green{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.15f, 0.75f, 0.15f) }) };

			// Load bunny mesh
			AssetLoader loader{};
			loader.Load("resources/models/bunny/lowpoly_bunny.obj",
				*this, false, mat_white, false);
			objects.at(0).instance_transformation =
				Matrix::CreateRotationY(static_cast<float>(std::numbers::pi)) * Matrix::CreateScale(2.f, 2.f, 2.f);

			// Cornell box walls (bounded, single-sided)
			constexpr bool ds{ false };
			const Vector2 fb_ext{ 5.f, 7.f };
			const Vector2 wall_ext{ 7.f, 5.f };
			const Vector2 back_ext{ 5.f, 5.f };
			const uint32_t p_floor = primitives_factory.Create<Plane>(
				Vector3(0.f, 0.f, 3.f), Vector3(0.f, 1.f, 0.f), ds, fb_ext);
			const uint32_t p_ceiling = primitives_factory.Create<Plane>(
				Vector3(0.f, 10.f, 3.f), Vector3(0.f, -1.f, 0.f), ds, fb_ext);
			const uint32_t p_back = primitives_factory.Create<Plane>(
				Vector3(0.f, 5.f, 10.f), Vector3(0.f, 0.f, -1.f), ds, back_ext);
			const uint32_t p_left = primitives_factory.Create<Plane>(
				Vector3(-5.f, 5.f, 3.f), Vector3(1.f, 0.f, 0.f), ds, wall_ext);
			const uint32_t p_right = primitives_factory.Create<Plane>(
				Vector3(5.f, 5.f, 3.f), Vector3(-1.f, 0.f, 0.f), ds, wall_ext);
			objects.push_back({ .primitive_index = p_floor, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_ceiling, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_back, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_left, .material_index = mat_red });
			objects.push_back({ .primitive_index = p_right, .material_index = mat_green });
		}

		bool Update(const double delta_time) override
		{
			if (!rotating_lights)
				return false;

			if (Light* light = lights_factory.GetAs<Light>(light_index))
			{
				light_time += static_cast<float>(delta_time) * 2.5f;
				constexpr float radius{ 4.f };
				constexpr float center_y{ 7.f };
				constexpr float center_z{ 5.f };
				light->origin = {
					std::cos(light_time) * radius,
					center_y,
					center_z + std::sin(light_time) * radius };
			}
			return true;
		}
	};

	// -------------------------------------------------------------------------
	// FlightHelmetScene: glTF FlightHelmet with directional light + HDRI.
	// Shared between path tracer and rasterizer. Enable ENABLE_HELMET_ROTATION
	// for the rasterizer to show the model spinning (disabled by default for PT).
	// -------------------------------------------------------------------------
	struct DamagedHelmetScene final : public Scene
	{
		float rotation_angle{ 0.f };
		bool rotating_model{ false };

		void Setup() override
		{
			// Camera setup
		#define CAMERA_SETUP 0
		#if CAMERA_SETUP == 0
			camera.SetPosition({ 0.564f, 0.696f, -0.980f });
			camera.SetFovAngle(45.f);
			camera.Pitch(-0.3260f);
			camera.Yaw(0.54f);
		#elif CAMERA_SETUP == 1
			camera.SetPosition({ 0.202f, 0.628f, -0.425f });
			camera.SetFovAngle(45.f);
			camera.Pitch(-0.3260f);
			camera.Yaw(0.4950f);
		#elif CAMERA_SETUP == 2
			camera.SetPosition({ -0.185f, 0.373f, -0.365f });
			camera.SetFovAngle(45.f);
			camera.Pitch(0.3820f);
			camera.Yaw(-0.5010f);
		#else
			camera.SetPosition({ -0.183f, 0.566f, -0.238f });
			camera.SetFovAngle(45.f);
			camera.Pitch(-0.0500f);
			camera.Yaw(-0.7170f);
		#endif

			// HDRI environment map
			environment_map.emplace(EnvironmentMap{
				.texture = Texture("resources/textures/hdri/papermill.hdr"),
				.intensity = 1.0f,
				.rotation_degrees = 180.f,
				.flip_horizontal = true });

			// Matches Khronos glTF Sample Viewer default punctual lights.
			// Directions derived from their quaternions, Z flipped for our coordinate system.
			// Intensities scaled by 2x to account for their exposure=1 (pow(2,1)=2).
			lights_factory.Create<Light>(Light{
				.direction = Vector3(0.5f, -0.7071f, 0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 2.f,
				.type = LightType::kDirectional });
			lights_factory.Create<Light>(Light{
				.direction = Vector3(-0.5f, 0.7071f, -0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 1.f,
				.type = LightType::kDirectional });

			// Load FlightHelmet glTF (materials auto-created from glTF)
			AssetLoader loader{};
			loader.Load("resources/models/flight_helmet/glTF/FlightHelmet.gltf", *this,
				false, std::numeric_limits<uint32_t>::max(),
				true, true);
		}

		bool Update(const double delta_time) override
		{
			if (!rotating_model)
				return false;

			constexpr float rotation_speed{ 2.85f };
			rotation_angle += static_cast<float>(delta_time * rotation_speed);
			const Matrix transform{ Matrix::CreateRotationY(rotation_angle, true) };
			for (SceneObject& object : objects)
				object.instance_transformation = transform;
			return true;
		}
	};

	// -------------------------------------------------------------------------
	// CornellBoxScene: Cornell box with PBR material spheres.
	// -------------------------------------------------------------------------
	struct CornellBoxScene final : public Scene
	{
		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0, 3, -9.f });
			camera.SetFovAngle(45.f);

			// Light setup
			lights_factory.Create<Light>(Light{
				.origin = {0.f, 9.5f, 5.f},
				.color = ColorRgba::White(),
				.intensity = 50.f,
				.type = LightType::kPoint });

			// Lambert material setup
			const uint32_t mat_white{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{.diffuse_color = ColorRgba(0.7f, 0.7f, 0.7f) }) };
			const uint32_t mat_red{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.75f, 0.15f, 0.15f) }) };
			const uint32_t mat_green{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{ .diffuse_color = ColorRgba(0.15f, 0.75f, 0.15f) }) }	;

			// PBR material setup
			const uint32_t mat_chrome{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.95f, 0.93f, 0.88f},
					.metallic_factor = 1.f, .roughness_factor = 0.05f }) };
			const uint32_t mat_rough_plastic{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.8f, 0.2f, 0.2f},
					.metallic_factor = 0.f, .roughness_factor = 0.6f }) };
			const uint32_t mat_gold{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {1.f, 0.782f, 0.344f},
					.metallic_factor = 1.f, .roughness_factor = 0.35f }) };

			// 3 PBR spheres
			const uint32_t s0 {primitives_factory.Create<Sphere>(Vector3(-2.5f, 1.5f, 5.f), 1.5f)};
			const uint32_t s1 {primitives_factory.Create<Sphere>(Vector3(0.f, 1.f, 3.f), 1.f)};
			const uint32_t s2 {primitives_factory.Create<Sphere>(Vector3(2.5f, 1.5f, 5.f), 1.5f)};
			objects.push_back({ .primitive_index = s0, .material_index = mat_chrome });
			objects.push_back({ .primitive_index = s1, .material_index = mat_rough_plastic });
			objects.push_back({ .primitive_index = s2, .material_index = mat_gold });

			// Cornell box walls (bounded, single-sided)
			constexpr bool ds{ false };
			const Vector2 fb_ext{ 5.f, 7.f };
			const Vector2 wall_ext{ 7.f, 5.f };
			const Vector2 back_ext{ 5.f, 5.f };
			const uint32_t p_floor{ primitives_factory.Create<Plane>(
				Vector3(0.f, 0.f, 3.f), Vector3(0.f, 1.f, 0.f), ds, fb_ext) };
			const uint32_t p_ceiling{ primitives_factory.Create<Plane>(
				Vector3(0.f, 10.f, 3.f), Vector3(0.f, -1.f, 0.f), ds, fb_ext) };
			const uint32_t p_back{ primitives_factory.Create<Plane>(
				Vector3(0.f, 5.f, 10.f), Vector3(0.f, 0.f, -1.f), ds, back_ext) };
			const uint32_t p_left{ primitives_factory.Create<Plane>(
				Vector3(-5.f, 5.f, 3.f), Vector3(1.f, 0.f, 0.f), ds, wall_ext) };
			const uint32_t p_right{ primitives_factory.Create<Plane>(
				Vector3(5.f, 5.f, 3.f), Vector3(-1.f, 0.f, 0.f), ds, wall_ext) };
			objects.push_back({ .primitive_index = p_floor, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_ceiling, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_back, .material_index = mat_white });
			objects.push_back({ .primitive_index = p_left, .material_index = mat_red });
			objects.push_back({ .primitive_index = p_right, .material_index = mat_green });
		}
	};

	// -------------------------------------------------------------------------
	// PathTraceScene: Path tracing showcase with HDRI environment.
	// -------------------------------------------------------------------------
	struct PathTraceScene final : public Scene
	{
		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0, 3.f, -7.f });
			camera.SetFovAngle(50.f);
			camera.Pitch(-0.25f);

			// Fallback sky color (visible at step 0 before HDRI is enabled)
			background_color = ColorRgba(0.5f, 0.7f, 1.0f);

			// HDRI environment map
			environment_map.emplace(EnvironmentMap{
				.texture = Texture("resources/textures/hdri/witsand_woolshop_4k.hdr"),
				.intensity = 1.0f });

			// Light setup
			lights_factory.Create<Light>(Light{
				.origin = {0.f, 8.f, -2.f},
				.color = {1.f, 0.95f, 0.85f},
				.intensity = 25.f,
				.type = LightType::kPoint });

			// PBR material setup
			const uint32_t mat_glass{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {1.f, 1.f, 1.f},
					.metallic_factor = 0.f, .roughness_factor = 0.f,
					.transmission_factor = 1.f, .ior = 1.5f }) };
			const uint32_t mat_rough_plastic{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.8f, 0.2f, 0.15f},
					.metallic_factor = 0.f, .roughness_factor = 0.6f }) };
			const uint32_t mat_polished_metal{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.95f, 0.93f, 0.88f},
					.metallic_factor = 1.f, .roughness_factor = 0.05f }) };
			const uint32_t mat_polished_plastic{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.15f, 0.35f, 0.75f},
					.metallic_factor = 0.f, .roughness_factor = 0.05f,
					.ior = 1.5f }) };
			const uint32_t mat_bg_sphere{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.9f, 0.9f, 0.1f},
					.metallic_factor = 0.f, .roughness_factor = 0.5f }) };

			// Ground plane (neutral gray PBR)
			const uint32_t mat_ground{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = {0.5f, 0.5f, 0.5f},
					.metallic_factor = 0.f, .roughness_factor = 1.0f }) };
			const uint32_t ground{ primitives_factory.Create<Plane>(
				Vector3(0.f, 0.f, 2.f), Vector3(0.f, 1.f, 0.f),
				false, Vector2(6.f, 6.f)) };
			objects.push_back({ .primitive_index = ground, .material_index = mat_ground });

			// 4 spheres in a row
			const uint32_t s0{ primitives_factory.Create<Sphere>(Vector3(-3.f, 0.8f, 0.f), 0.8f) };
			const uint32_t s1{ primitives_factory.Create<Sphere>(Vector3(-1.f, 0.8f, 0.f), 0.8f) };
			const uint32_t s2{ primitives_factory.Create<Sphere>(Vector3(1.f, 0.8f, 0.f), 0.8f) };
			const uint32_t s3{ primitives_factory.Create<Sphere>(Vector3(3.f, 0.8f, 0.f), 0.8f) };
			objects.push_back({ .primitive_index = s0, .material_index = mat_glass });
			objects.push_back({ .primitive_index = s1, .material_index = mat_rough_plastic });
			objects.push_back({ .primitive_index = s2, .material_index = mat_polished_metal });
			objects.push_back({ .primitive_index = s3, .material_index = mat_polished_plastic });

			// Small yellow sphere behind glass sphere to showcase transparency
			const uint32_t bs0{ primitives_factory.Create<Sphere>(Vector3(-3.f, 0.4f, 1.8f), 0.35f) };
			objects.push_back({ .primitive_index = bs0, .material_index = mat_bg_sphere });
		}
	};

	// -------------------------------------------------------------------------
	// DragonAttenuationScene: glTF dragon with transmission and volume
	// attenuation, showcasing KHR_materials_volume/transmission extensions.
	// -------------------------------------------------------------------------
	struct DragonAttenuationScene final : public Scene
	{
		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0.598f, 1.643f, -5.160f });
			camera.SetFovAngle(45.f);
			camera.Pitch(-0.2040f);
			camera.Yaw(0.0930f);

			// HDRI environment map
			environment_map.emplace(EnvironmentMap{
				.texture = Texture("resources/textures/hdri/papermill.hdr"),
				.intensity = 1.0f,
				.flip_horizontal = true });

			// Directional lights (matching Khronos sample viewer defaults)
			lights_factory.Create<Light>(Light{
				.direction = Vector3(0.5f, -0.7071f, 0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 2.f,
				.type = LightType::kDirectional });
			lights_factory.Create<Light>(Light{
				.direction = Vector3(-0.5f, 0.7071f, -0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 1.f,
				.type = LightType::kDirectional });

			// Load DragonAttenuation glTF
			AssetLoader loader{};
			loader.Load("resources/models/dragon_attenuation/glTF/DragonAttenuation.gltf",
				*this, false, std::numeric_limits<uint32_t>::max(),
				true, true);
		}
	};

	// -------------------------------------------------------------------------
	// ChessScene: "A Beautiful Game" glTF chess set with glass pawn tops.
	// Uses KHR_materials_transmission for path-traced glass refraction.
	// -------------------------------------------------------------------------
	struct ChessScene final : public Scene
	{
		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0.244f, 0.265f, -0.338f });
			camera.SetFovAngle(45.f);
			camera.Pitch(-0.5390f);
			camera.Yaw(0.6300f);

			// HDRI environment map
			environment_map.emplace(EnvironmentMap{
				.texture = Texture("resources/textures/hdri/papermill.hdr"),
				.intensity = 1.0f });

			// Directional light
			lights_factory.Create<Light>(Light{
				.direction = Vector3(-0.5f, -1.f, 0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 2.f,
				.type = LightType::kDirectional });

			// Load chess set glTF (includes transmission on glass pawn tops)
			AssetLoader loader{};
			loader.Load("resources/models/a_beautiful_game/glTF/ABeautifulGame.gltf",
				*this, false, std::numeric_limits<uint32_t>::max(),
				true, true);
		}
	};

	// -------------------------------------------------------------------------
	// WhiteFurnaceScene: Energy conservation test.
	// White spheres in a uniform white environment. If the BRDF is
	// energy-conserving, each sphere should be indistinguishable from 
	// the background.
	// -------------------------------------------------------------------------
	struct WhiteFurnaceScene final : public Scene
	{
		void Setup() override
		{
			camera.SetPosition({ 0, 0, -7.f });
			camera.SetFovAngle(45.f);
			background_color = ColorRgba::White();

			// Left: pure Lambert (should vanish completely)
			const uint32_t mat_lambert{ materials_factory.Create<LambertMaterial>(
				LambertMaterialParams{.diffuse_color = ColorRgba::White(),
					.diffuse_reflectance = 1.f }) };
			const uint32_t sphere_l{ primitives_factory.Create<Sphere>(
				Vector3(-2.5f, 0.f, 0.f), 1.f) };
			objects.push_back({ .primitive_index = sphere_l, .material_index = mat_lambert });

			// Center: PBR dielectric, fully rough (mostly diffuse path)
			const uint32_t mat_pbr_rough{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{.albedo_color_factor = ColorRgba::White(),
					.metallic_factor = 0.f, .roughness_factor = 1.f }) };
			const uint32_t sphere_c{ primitives_factory.Create<Sphere>(
				Vector3(0.f, 0.f, 0.f), 1.f) };
			objects.push_back({ .primitive_index = sphere_c, .material_index = mat_pbr_rough });

			// Right: PBR metallic, very smooth (pure specular mirror)
			const uint32_t mat_pbr_smooth{ materials_factory.Create<PbrMaterial>(
				PbrMaterialParams{ .albedo_color_factor = ColorRgba::White(),
					.metallic_factor = 1.f, .roughness_factor = 0.05f }) };
			const uint32_t sphere_r{ primitives_factory.Create<Sphere>(
				Vector3(2.5f, 0.f, 0.f), 1.f) };
			objects.push_back({ .primitive_index = sphere_r, .material_index = mat_pbr_smooth });
		}
	};

	// =========================================================================
	// RASTERIZER SCENES
	// =========================================================================
	// -------------------------------------------------------------------------
	// NdcTriangleScene: Single triangle in NDC (no WVP transform needed).
	// Used at step 2 (position only, colors ignored) and step 3 (rainbow).
	// -------------------------------------------------------------------------
	struct NdcTriangleScene final : public Scene
	{
		void Setup() override
		{
			const uint32_t mat_unlit{ materials_factory.Create<UnlitMaterial>(
				UnlitMaterialParams{ .color = ColorRgba::White() }) };

			const std::vector<Vertex> vertices
			{
				{.position = Vector4(0.f, .5f, 1.f, 1.f),
					.color = Vector3(1.f, 0.f, 0.f) },
				{.position = Vector4(.5f, -.5f, 1.f, 1.f),
					.color = Vector3(0.f, 1.f, 0.f) },
				{.position = Vector4(-.5f, -.5f, 1.f, 1.f),
					.color = Vector3(0.f, 0.f, 1.f) }
			};
			const std::vector<uint32_t> indices{ 0, 1, 2 };
			const uint32_t tri{ primitives_factory.Create<TriangleMesh>(
				vertices, indices, CullMode::kNoCulling) };
			objects.push_back({ .primitive_index = tri, .material_index = mat_unlit });
		}
	};

	// -------------------------------------------------------------------------
	// WorldTriangleScene: Single triangle in world space (requires WVP).
	// -------------------------------------------------------------------------
	struct WorldTriangleScene final : public Scene
	{
		void Setup() override
		{
			camera.SetPosition({ 0, 0, -10.f });
			camera.SetFovAngle(60.f);

			const uint32_t mat_unlit{ materials_factory.Create<UnlitMaterial>(
				UnlitMaterialParams{ .color = ColorRgba::White() }) };

			const std::vector<Vertex> vertices
			{
				{.position = Vector4(0.f, 2.f, 0.f, 1.f) },
				{.position = Vector4(1.f, 0.f, 0.f, 1.f) },
				{.position = Vector4(-1.f, 0.f, 0.f, 1.f) }
			};
			const std::vector<uint32_t> indices{ 0, 1, 2 };
			const uint32_t tri{ primitives_factory.Create<TriangleMesh>(
				vertices, indices, CullMode::kNoCulling) };
			objects.push_back({ .primitive_index = tri, .material_index = mat_unlit });
		}
	};

	// -------------------------------------------------------------------------
	// ColorTriangleScene: Vertex-colored triangle (barycentric interpolation).
	// -------------------------------------------------------------------------
	struct ColorTriangleScene final : public Scene
	{
		void Setup() override
		{
			camera.SetPosition({ 0, 0, -10.f });
			camera.SetFovAngle(60.f);

			const uint32_t mat_unlit{ materials_factory.Create<UnlitMaterial>(
				UnlitMaterialParams{ .color = ColorRgba::White() }) };

			// Front triangle (red): smaller, closer.
			const std::vector<Vertex> vertices_0
			{
				{.position = Vector4(0.f, 2.f, 0.f, 1.f),
					.color = Vector3(1.f, 0.f, 0.f),
					.texture_coordinates = Vector2(0.5f, 0.f) },
				{.position = Vector4(1.5f, -1.f, 0.f, 1.f),
					.color = Vector3(1.f, 0.f, 0.f),
					.texture_coordinates = Vector2(1.f, 1.f) },
				{.position = Vector4(-1.5f, -1.f, 0.f, 1.f),
					.color = Vector3(1.f, 0.f, 0.f),
					.texture_coordinates = Vector2(0.f, 1.f) }
			};
			// Back triangle (RGB gradient): larger, behind.
			const std::vector<Vertex> vertices_1
			{
				{.position = Vector4(0.f, 4.f, 2.f, 1.f),
					.color = Vector3(1.f, 0.f, 0.f),
					.texture_coordinates = Vector2(0.5f, 0.f) },
				{.position = Vector4(3.f, -2.f, 2.f, 1.f),
					.color = Vector3(0.f, 1.f, 0.f),
					.texture_coordinates = Vector2(1.f, 1.f) },
				{.position = Vector4(-3.f, -2.f, 2.f, 1.f),
					.color = Vector3(0.f, 0.f, 1.f),
					.texture_coordinates = Vector2(0.f, 1.f) }
			};
			const std::vector<uint32_t> indices{ 0, 1, 2 };

			const uint32_t mesh_0{ primitives_factory.Create<TriangleMesh>(
				vertices_0, indices, CullMode::kNoCulling) };
			const uint32_t mesh_1{ primitives_factory.Create<TriangleMesh>(
				vertices_1, indices, CullMode::kNoCulling) };
			objects.push_back({ .primitive_index = mesh_0, .material_index = mat_unlit });
			objects.push_back({ .primitive_index = mesh_1, .material_index = mat_unlit });
		}
	};

	// -------------------------------------------------------------------------
	// QuadScene: Two 3x3 vertex grids side by side, each using a different
	// primitive topology. Left = triangle list, Right = triangle strip.
	// Both produce the same visual result — the topology only changes how
	// indices reference the shared vertex buffer.
	// -------------------------------------------------------------------------
	struct QuadScene final : public Scene
	{
		void Setup() override
		{
		#define SHOWCASE_DISTORTION_VIEW 0
		#if SHOWCASE_DISTORTION_VIEW == 1
			camera.SetPosition({ 6.866f, -0.003f, -5.599f });
			camera.SetFovAngle(60.0f);
			camera.Pitch(-0.0090f);
			camera.Yaw(0.8730f);
			camera.SetFovAngle(60.f);
		#else
			camera.SetPosition({ 0, 0, -10.f });
			camera.SetFovAngle(60.f);
		#endif

			const uint32_t debug_texture{ textures_factory.Create<Texture>(
				"resources/textures/debug_textures/uv_grid_2.png") };
			const uint32_t mat_textured{ materials_factory.Create<UnlitMaterial>(
				UnlitMaterialParams{
					.color = ColorRgba::White(),
					.color_texture = debug_texture }) };

			// 3x3 vertex grid with UVs (shared by both meshes).
			// 0---1---2
			// | / | / |
			// 3---4---5
			// | / | / |
			// 6---7---8
			const std::vector<Vertex> vertices
			{
				{.position = Vector4(-2.f,  2.f, 0.f, 1.f), .texture_coordinates = Vector2(0.f, 0.f) },
				{.position = Vector4(0.f,  2.f, 0.f, 1.f), .texture_coordinates = Vector2(0.5f, 0.f) },
				{.position = Vector4(2.f,  2.f, 0.f, 1.f), .texture_coordinates = Vector2(1.f, 0.f) },
				{.position = Vector4(-2.f,  0.f, 0.f, 1.f), .texture_coordinates = Vector2(0.f, 0.5f) },
				{.position = Vector4(0.f,  0.f, 0.f, 1.f), .texture_coordinates = Vector2(0.5f, 0.5f) },
				{.position = Vector4(2.f,  0.f, 0.f, 1.f), .texture_coordinates = Vector2(1.f, 0.5f) },
				{.position = Vector4(-2.f, -2.f, 0.f, 1.f), .texture_coordinates = Vector2(0.f, 1.f) },
				{.position = Vector4(0.f, -2.f, 0.f, 1.f), .texture_coordinates = Vector2(0.5f, 1.f) },
				{.position = Vector4(2.f, -2.f, 0.f, 1.f), .texture_coordinates = Vector2(1.f, 1.f) }
			};

			// Left: Triangle list — each 3 indices form one independent triangle.
			// Winding produces positive edge-function cross products in screen
			// space, matching OpenGL/Vulkan CCW front-face convention.
			const std::vector<uint32_t> list_indices {
				3, 0, 4,  0, 1, 4,   // Top-left quad
				4, 1, 5,  1, 2, 5,   // Top-right quad
				6, 3, 7,  3, 4, 7,   // Bottom-left quad
				7, 4, 8,  4, 5, 8 }; // Bottom-right quad
			const uint32_t list_mesh {primitives_factory.Create<TriangleMesh>(
				vertices, list_indices, CullMode::kNoCulling, PrimitiveTopology::kTriangleList) };
			objects.push_back({
				.primitive_index = list_mesh,
				.material_index = mat_textured,
				.instance_transformation = Matrix::CreateTranslation(-3.f, 0.f, -2.f) });

			// Right: Triangle strip — each new vertex forms a triangle with
			// the previous two. Degenerate triangles (repeated indices) restart
			// the strip for row 2.
			// Row 1: 3-0-4-1-5-2  |  Degenerates: 2-6  |  Row 2: 6-3-7-4-8-5
			const std::vector<uint32_t> strip_indices {
				3, 0, 4, 1, 5, 2, 2,
				6, 6, 3, 7, 4, 8, 5 };
			const uint32_t strip_mesh {primitives_factory.Create<TriangleMesh>(
				vertices, strip_indices, CullMode::kNoCulling, PrimitiveTopology::kTriangleStrip) };
			objects.push_back({
				.primitive_index = strip_mesh,
				.material_index = mat_textured,
				.instance_transformation = Matrix::CreateTranslation(3.f, 0.f, -2.f) });
		}
	};

	// -------------------------------------------------------------------------
	// AlphaBlendModeTestScene: Khronos glTF alpha conformance test model.
	// Showcases kMask (alpha cutout) and kBlend (alpha blend) modes.
	// -------------------------------------------------------------------------
	struct AlphaBlendModeTestScene final : public Scene
	{
		void Setup() override
		{
			// Camera setup
			camera.SetPosition({ 0.25f, 3.80f, -7.85f });
			camera.Pitch(-0.3330f);
			camera.Yaw(0.0210f);
			camera.SetFovAngle(45.f);

			// Directional lights
			lights_factory.Create<Light>(Light{
				.direction = Vector3(-0.5f, -1.f, 0.5f).Normalized(),
				.color = ColorRgba::White(),
				.intensity = 1.5f,
				.type = LightType::kDirectional });

			// Load AlphaBlendModeTest glTF (uses explicit alphaMode: MASK and BLEND)
			AssetLoader loader{};
			loader.Load("resources/models/alpha_blend_mode_test/glTF/AlphaBlendModeTest.gltf",
				*this, false, std::numeric_limits<uint32_t>::max(),
				true, true);
		}
	};
}

#endif //SCENES_HEADER