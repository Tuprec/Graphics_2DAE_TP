//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#ifndef DEBUG_VIEWS_HEADER
#define DEBUG_VIEWS_HEADER

//--- Standard Includes ---
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <vector>

//--- Framework Includes ---
#include <color.h>

namespace gfx
{
	//--- Enums ---
	enum class VisualizationMode : uint8_t
	{
		kNone,
		kNormals,
		kAlbedo,
		kDepth,
		kUV,
		kMetallicRoughness,
		kDirectOnly,
		kBounceCount,
		kMax
	};

	enum class FilterMode : uint8_t
	{
		kPoint,
		kLinear,
		kAnisotropic,
		kMax
	};

	enum class SamplingStrategy : uint8_t
	{
		kUniform,
		kCosine,
		kGGX,
		kMax
	};

	//--- Debug Parameters ---
	struct DebugParams final
	{
		//VisualizationMode visualization_mode{ VisualizationMode::kNone };
		VisualizationMode visualization_mode{ VisualizationMode::kNormals };
		//VisualizationMode visualization_mode{ VisualizationMode::kDepth };
		SamplingStrategy sampling_strategy{ SamplingStrategy::kGGX };
		ToneMapOperator tone_map_operator{ ToneMapOperator::kACESHill };
		FilterMode filter_mode{ FilterMode::kPoint };
		uint32_t max_samples{ std::numeric_limits<uint32_t>::max() };
		uint32_t accumulated_samples{ 0 };
		bool sampling_paused{ false };
		bool show_denoised{ false };
		bool update_active_scene{ true };
		bool print_timer{ false };
		bool show_bounding_boxes{ false };
	};

	//--- Key Bindings ---
	struct KeyBinding final
	{
		int key;
		const char* description;
		std::function<void()> action;
	};

	// Forward declarations for users
	struct Context;
	class Logger;
	std::vector<KeyBinding> RegisterKeyBindings(Context& context);
	void PrintKeyBindings(const std::vector<KeyBinding>& bindings, const Logger& logger);

	//--- String Conversion Helpers ---
	inline std::string VisualizationModeToString(const VisualizationMode& mode)
	{
		switch (mode)
		{
		case VisualizationMode::kNone:
			return "None";
		case VisualizationMode::kNormals:
			return "Normals";
		case VisualizationMode::kAlbedo:
			return "Albedo";
		case VisualizationMode::kDepth:
			return "Depth";
		case VisualizationMode::kUV:
			return "UV";
		case VisualizationMode::kMetallicRoughness:
			return "Metallic/Roughness";
		case VisualizationMode::kDirectOnly:
			return "Direct Only";
		case VisualizationMode::kBounceCount:
			return "Bounce Count";
		case VisualizationMode::kMax:
			return "";
		}
		return "";
	}

	inline std::string SamplingStrategyToString(const SamplingStrategy& strategy)
	{
		switch (strategy)
		{
		case SamplingStrategy::kUniform:
			return "Uniform Hemisphere";
		case SamplingStrategy::kCosine:
			return "Cosine-Weighted Hemisphere";
		case SamplingStrategy::kGGX:
			return "GGX Importance Sampling";
		case SamplingStrategy::kMax:
			return "";
		}
		return "";
	}

	inline std::string FilterModeToString(const FilterMode& mode)
	{
		switch (mode)
		{
		case FilterMode::kPoint:
			return "Point";
		case FilterMode::kLinear:
			return "Linear";
		case FilterMode::kAnisotropic:
			return "Anisotropic";
		case FilterMode::kMax:
			return "";
		}
		return "";
	}

	inline std::string ToneMapOperatorToString(const ToneMapOperator& op)
	{
		switch (op)
		{
		case ToneMapOperator::kNone:
			return "None";
		case ToneMapOperator::kReinhard:
			return "Reinhard";
		case ToneMapOperator::kACESNarkowicz:
			return "ACES (Narkowicz)";
		case ToneMapOperator::kACESHill:
			return "ACES (Hill)";
		case ToneMapOperator::kMax:
			return "";
		}
		return "";
	}
}

#endif //DEBUG_VIEWS_HEADER
