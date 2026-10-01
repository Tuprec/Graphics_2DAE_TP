//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef ASSET_LOADER_HEADER
#define ASSET_LOADER_HEADER

//--- Standard Includes ---
#include <string>
#include <unordered_map>

//--- Framework Includes ---
#include <scene_manager.h>

//--- Forward Declarations ---
struct aiMaterial;

namespace gfx
{
	class AssetLoader final
	{
		//--- Data Members ---
		std::unordered_map<std::string, uint32_t> loaded_textures_map_{};

		//--- Functions ---
		uint32_t CreateTextureUnique(Scene& scene, const std::string& path, bool is_srgb = false);
		uint32_t CreateMaterial(Scene& scene, const aiMaterial* ai_material,
			const std::string& model_directory, bool auto_detect_alpha = false);

	public:
		//--- Functions ---
		bool Load(const std::string& path, Scene& scene, bool use_instancing_transforms = false,
			const uint32_t override_material_index = std::numeric_limits<uint32_t>::max(),
			const bool convert_to_left_handed = true,
			bool auto_detect_alpha = false);
	};
}
#endif //ASSET_LOADER_HEADER