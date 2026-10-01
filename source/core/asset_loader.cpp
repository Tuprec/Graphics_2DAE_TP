//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include "asset_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <filesystem>
#include <functional>
#include <vector>
#include <unordered_map>
using namespace gfx;

uint32_t AssetLoader::CreateTextureUnique(Scene& scene, const std::string& path, const bool is_srgb)
{
	const auto it{ loaded_textures_map_.find(path) };
	if (it != loaded_textures_map_.end())
		return it->second;

	const uint32_t texture_index = scene.textures_factory.Create<Texture>(path, is_srgb);
	loaded_textures_map_[path] = texture_index;
	return texture_index;
}

uint32_t AssetLoader::CreateMaterial(Scene& scene, const aiMaterial* ai_material,
	const std::string& model_directory, const bool auto_detect_alpha)
{
	// Colors
	aiColor4D base_color(1.f, 1.f, 1.f, 1.f);
	aiColor4D diffuse_color(1.f, 1.f, 1.f, 1.f);
	// For PBR try and load the base color, but if fails fallback to diffuse.
	if (ai_material->Get(AI_MATKEY_BASE_COLOR, base_color) != AI_SUCCESS)
		ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color);
	ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);

	// Get other data.
	float metallic {0.f}, roughness {1.f};
	ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
	ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

	// Check if textures are provided.
	bool has_diffuse_texture = ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0;
	bool has_normal_texture = ai_material->GetTextureCount(aiTextureType_NORMALS) > 0;
	bool has_base_color_texture = ai_material->GetTextureCount(aiTextureType_BASE_COLOR) > 0;
	bool has_metallic_roughness_texture = ai_material->GetTextureCount(aiTextureType_METALNESS) > 0
		|| ai_material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0
		|| ai_material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) > 0;
	aiString texture_path = {};

	// --- PBR ---
	if (metallic > 0.f || roughness < 1.f || has_base_color_texture || has_metallic_roughness_texture)
	{
		PbrMaterialParams pbr_params{};
		pbr_params.albedo_color_factor = { base_color.r, base_color.g, base_color.b, base_color.a };
		pbr_params.metallic_factor = metallic;
		pbr_params.roughness_factor = roughness;

		// Base color texture - with fallback (sRGB encoded).
		if (has_base_color_texture && ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path) == AI_SUCCESS)
			pbr_params.albedo_color_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string(), true);
		else if (has_diffuse_texture && ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == AI_SUCCESS)
			pbr_params.albedo_color_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string(), true);

		// Metallic-Roughness packed texture
		if (has_metallic_roughness_texture && ai_material->GetTexture(aiTextureType_METALNESS, 0, &texture_path) == AI_SUCCESS)
			pbr_params.metallic_roughness_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());
		else if (has_metallic_roughness_texture && ai_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texture_path) == AI_SUCCESS)
			pbr_params.metallic_roughness_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());
		else if (has_metallic_roughness_texture && ai_material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texture_path) == AI_SUCCESS)
			pbr_params.metallic_roughness_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());

		// Normal Texture (could be stored in aiTextureType_HEIGHT or aiTextureType_DISPLACEMENT as well...)
		if (has_normal_texture && ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == AI_SUCCESS)
			pbr_params.normal_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());

		// Occlusion Texture (Assimp maps glTF occlusionTexture to aiTextureType_LIGHTMAP)
		if (ai_material->GetTextureCount(aiTextureType_LIGHTMAP) > 0
			&& ai_material->GetTexture(aiTextureType_LIGHTMAP, 0, &texture_path) == AI_SUCCESS)
			pbr_params.occlusion_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());

		// Alpha mode (glTF: OPAQUE, MASK, BLEND)
		bool has_explicit_alpha_mode {false};
		aiString alpha_mode_str{};
		if (ai_material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode_str) == AI_SUCCESS)
		{
			has_explicit_alpha_mode = true;
			if (std::string(alpha_mode_str.C_Str()) == "MASK")
			{
				pbr_params.alpha_params.mode = AlphaMode::kMask;
				float alpha_cutoff = 0.5f;
				ai_material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alpha_cutoff);
				pbr_params.alpha_params.cutoff = alpha_cutoff;
			}
			else if (std::string(alpha_mode_str.C_Str()) == "BLEND")
				pbr_params.alpha_params.mode = AlphaMode::kBlend;
		}

		// KHR_materials_transmission: treat as blend for rasterizer.
		float transmission_factor {0.f};
		if (ai_material->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission_factor) == AI_SUCCESS
			&& transmission_factor > 0.f)
		{
			pbr_params.transmission_factor = transmission_factor;
			if (pbr_params.alpha_params.mode == AlphaMode::kOpaque)
				pbr_params.alpha_params.mode = AlphaMode::kBlend;
		}

		// KHR_materials_ior
		float ior{ 0.f };
		if (ai_material->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS && ior > 0.f)
			pbr_params.ior = ior;

		// KHR_materials_volume
		float thickness_factor{ 0.f };
		if (ai_material->Get(AI_MATKEY_VOLUME_THICKNESS_FACTOR, thickness_factor) == AI_SUCCESS)
			pbr_params.thickness_factor = thickness_factor;
		float attenuation_distance {0.f};
		if (ai_material->Get(AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, attenuation_distance) == AI_SUCCESS
			&& attenuation_distance > 0.f)
			pbr_params.attenuation_distance = attenuation_distance;
		aiColor4D attenuation_color{};
		if (ai_material->Get(AI_MATKEY_VOLUME_ATTENUATION_COLOR, attenuation_color) == AI_SUCCESS)
			pbr_params.attenuation_color = { attenuation_color.r, attenuation_color.g, attenuation_color.b };
		if (ai_material->GetTexture(AI_MATKEY_VOLUME_THICKNESS_TEXTURE, &texture_path) == AI_SUCCESS)
			pbr_params.thickness_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());

		// Auto-detect: only when format has no explicit alphaMode (e.g. OBJ/FBX).
		// If the glTF explicitly says OPAQUE, respect that even if the texture has alpha.
		if (auto_detect_alpha && !has_explicit_alpha_mode
			&& pbr_params.alpha_params.mode == AlphaMode::kOpaque
			&& pbr_params.albedo_color_texture.has_value())
		{
			const Texture* albedo_tex{ scene.textures_factory.Get(
				pbr_params.albedo_color_texture.value()) };
			if (albedo_tex && albedo_tex->GetChannels() == 4)
				pbr_params.alpha_params.mode = AlphaMode::kBlend;
		}

		// Create material.
		return scene.materials_factory.Create<PbrMaterial>(pbr_params);
	}

	// --- Lambert ---
	if (diffuse_color.r != 0.f || diffuse_color.g != 0.f || diffuse_color.b != 0.f || has_diffuse_texture)
	{
		LambertMaterialParams lambert_params = {};
		lambert_params.diffuse_color = { diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a };

		// Textures (sRGB encoded).
		if (has_diffuse_texture && ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == AI_SUCCESS)
			lambert_params.diffuse_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string(), true);

		// Normal Texture (could be stored in aiTextureType_HEIGHT or aiTextureType_DISPLACEMENT as well...)
		if (has_normal_texture && ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == AI_SUCCESS)
			lambert_params.normal_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string());

		return scene.materials_factory.Create<LambertMaterial>(lambert_params);
	}

	//--- Unlit --- [fallback]
	UnlitMaterialParams unlit_params{};
	unlit_params.color = { diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a };
	if (has_diffuse_texture && ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == AI_SUCCESS)
		unlit_params.color_texture = CreateTextureUnique(scene, (std::filesystem::path(model_directory) / texture_path.C_Str()).string(), true);
	return scene.materials_factory.Create<UnlitMaterial>(unlit_params);
}

bool AssetLoader::Load(const std::string& path, Scene& scene,
	const bool use_instancing_transforms, const uint32_t override_material_index,
	const bool convert_to_left_handed, const bool auto_detect_alpha)
{
	// Check if file exists!
	const std::filesystem::path absolute_path{ std::filesystem::absolute(path) };
	if (!std::filesystem::exists(absolute_path))
		return false;

	// Extract model directory for texture path resolution.
	const std::string model_directory{ absolute_path.parent_path().string() };

	// Create assimp importer - TODO: fix it to work with instancing transforms.
	// Currently, the vertices get pretransformed so the hierarchy gets calculated for
	// the root transformation.
	Assimp::Importer importer{};
	//importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 180.f);
	unsigned int flags{ (aiProcessPreset_TargetRealtime_Fast & ~aiProcess_GenNormals)
		| aiProcess_GenSmoothNormals };
	if (convert_to_left_handed)
		flags |= aiProcess_ConvertToLeftHanded;
	if (!use_instancing_transforms)
		flags |= aiProcess_PreTransformVertices;
	const aiScene* ai_scene{ importer.ReadFile(absolute_path.string().c_str(), flags) };

	if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
		return false;

	// Iterate through all meshes.
	std::function<void(aiNode*, const aiMatrix4x4&)> ProcessNode =
		[&](const aiNode* node, const aiMatrix4x4& parent_transform)
		{
			const aiMatrix4x4 node_transform{ parent_transform * node->mTransformation };

			for (uint32_t i = 0; i < node->mNumMeshes; ++i)
			{
				// Get mesh data.
				const aiMesh* const mesh{ ai_scene->mMeshes[node->mMeshes[i]] };

				// Vertices
				std::vector<Vertex> vertices(mesh->mNumVertices);
				for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
				{
					aiVector3D pos{ mesh->mVertices[v] };
					if (!use_instancing_transforms)
						pos *= node_transform;
					vertices[v].position = { pos.x, pos.y, pos.z, 1.f };

					if (mesh->HasNormals())
					{
						aiMatrix3x3 rot(node_transform);
						aiVector3D n{ mesh->mNormals[v] };
						if (!use_instancing_transforms)
						{
							rot.Inverse().Transpose();
							n *= rot; n.Normalize();
						}
						vertices[v].normal = { n.x, n.y, n.z };
					}

					if (mesh->HasTangentsAndBitangents())
					{
						aiMatrix3x3 rot(node_transform);
						aiVector3D t{ mesh->mTangents[v] };
						aiVector3D b{ mesh->mBitangents[v] };
						if (!use_instancing_transforms)
						{
							rot.Inverse().Transpose();
							t *= rot; t.Normalize(); b *= rot; b.Normalize();
						}

						vertices[v].tangent = { t.x, t.y, t.z };
						vertices[v].bitangent = { b.x, b.y, b.z };

						const float handedness = (Vector3::Dot(Vector3::Cross(vertices[v].normal.value(), vertices[v].tangent.value()),
							vertices[v].bitangent.value()) > 0.f) ? -1.f : 1.f;
						if (handedness < 0.f)
							vertices[v].bitangent.value() *= -1.f;
					}

					if (mesh->HasTextureCoords(0))
					{
						const aiVector3D uv{ mesh->mTextureCoords[0][v] };

						// Wrap around uv coordinates on cpu.
						const bool is_gltf = (absolute_path.extension() == ".gltf" || absolute_path.extension() == ".glb");
						float uvx = is_gltf ? fmod(uv.x, 1.0f) : uv.x;
						float uvy = is_gltf ? fmod(uv.y, 1.0f) : uv.y;
						vertices[v].texture_coordinates = { uvx, uvy };
					}

					if (mesh->HasVertexColors(0))
					{
						aiColor4D c{ mesh->mColors[0][v] };
						vertices[v].color = { c.r, c.g, c.b };
					}
				}

				// Indices
				std::vector<uint32_t> indices{};
				indices.reserve(mesh->mNumFaces * 3);
				for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
				{
					const aiFace& face{ mesh->mFaces[f] };
					indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
				}

				// Create a SceneObject.
				SceneObject& scene_object{ scene.objects.emplace_back() };

				// Create primitive - TODO: figure out how to do instancing properly (need node_transform for this).
				scene_object.primitive_index = scene.primitives_factory.Create<TriangleMesh>(vertices, indices);

				// Create material if any.
				if ((mesh->mMaterialIndex < ai_scene->mNumMaterials) && (override_material_index == std::numeric_limits<uint32_t>::max()))
					scene_object.material_index = CreateMaterial(scene, ai_scene->mMaterials[mesh->mMaterialIndex], model_directory, auto_detect_alpha);
				else
					scene_object.material_index = override_material_index;
			}

			// Recursively process children.
			for (unsigned int c = 0; c < node->mNumChildren; ++c)
				ProcessNode(node->mChildren[c], node_transform);
		};

	// Kick off loading.
	ProcessNode(ai_scene->mRootNode, aiMatrix4x4());
	return true;
}
