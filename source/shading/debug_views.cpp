//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
#include <context.h>
#include <scenes.h>

namespace gfx
{
	std::vector<KeyBinding> RegisterKeyBindings(Context& context)
	{
		std::vector<KeyBinding> bindings;

		// --- Shared ---
		bindings.push_back({ SDL_SCANCODE_F1, "F1: Previous Scene", [&context]()
		{
			if (context.scene_manager)
			{
				context.scene_manager->PreviousScene();
				context.logger->LogInfo("Changed to previous scene.");
			}
		}});
		bindings.push_back({ SDL_SCANCODE_F2, "F2: Next Scene", [&context]()
		{
			if (context.scene_manager)
			{
				context.scene_manager->NextScene();
				context.logger->LogInfo("Changed to next scene.");
			}
		}});

		// --- Path Tracer ---
		bindings.push_back({ SDL_SCANCODE_F3, "F3: Previous Visualization Mode", [&context]()
		{
			using UType = std::underlying_type_t<VisualizationMode>;
			constexpr UType max = static_cast<UType>(VisualizationMode::kMax);
			context.debug_params.visualization_mode = static_cast<VisualizationMode>(
				(static_cast<UType>(context.debug_params.visualization_mode) + max - 1) % max);
			context.logger->LogInfo("Switched to Visualization Mode: {}",
				VisualizationModeToString(context.debug_params.visualization_mode));
		}});
		bindings.push_back({ SDL_SCANCODE_F4, "F4: Next Visualization Mode", [&context]()
		{
			using UType = std::underlying_type_t<VisualizationMode>;
			context.debug_params.visualization_mode = static_cast<VisualizationMode>(
				(static_cast<UType>(context.debug_params.visualization_mode) + 1) % static_cast<UType>(VisualizationMode::kMax));
			context.logger->LogInfo("Switched to Visualization Mode: {}",
				VisualizationModeToString(context.debug_params.visualization_mode));
		}});
		bindings.push_back({ SDL_SCANCODE_F5, "F5: Cycle Sampling Strategy (Uniform / Cosine / GGX)", [&context]()
		{
			using UType = std::underlying_type_t<SamplingStrategy>;
			context.debug_params.sampling_strategy = static_cast<SamplingStrategy>(
				(static_cast<UType>(context.debug_params.sampling_strategy) + 1) % static_cast<UType>(SamplingStrategy::kMax));
			context.logger->LogInfo("Switched to Sampling Strategy: {}",
				SamplingStrategyToString(context.debug_params.sampling_strategy));
		}});
		bindings.push_back({ SDL_SCANCODE_F6, "F6: Cycle Tone Mapper (None / Reinhard / ACES)", [&context]()
		{
			using UType = std::underlying_type_t<ToneMapOperator>;
			context.debug_params.tone_map_operator = static_cast<ToneMapOperator>(
				(static_cast<UType>(context.debug_params.tone_map_operator) + 1) % static_cast<UType>(ToneMapOperator::kMax));
			context.logger->LogInfo("Switched to Tone Mapper: {}",
				ToneMapOperatorToString(context.debug_params.tone_map_operator));
		}});
		bindings.push_back({ SDL_SCANCODE_F7, "F7: Pause/Resume Sampling", [&context]()
		{
			context.debug_params.sampling_paused = !context.debug_params.sampling_paused;
			context.logger->LogInfo("Sampling: {}", context.debug_params.sampling_paused ? "Paused" : "Running");
		}});

		// --- Rasterizer ---
		bindings.push_back({ SDL_SCANCODE_F8, "F8: Cycle Cull Modes (All Triangle Primitives)", [&context]()
		{
			Scene* active_scene = context.scene_manager ? context.scene_manager->GetActiveScene() : nullptr;
			if (active_scene)
			{
				using UType = std::underlying_type_t<CullMode>;
				constexpr UType max = static_cast<UType>(CullMode::kNoCulling) + 1;
				for (Triangle* triangle : active_scene->primitives_factory.GetAllOfType<Triangle>())
				{
					triangle->cull_mode = static_cast<CullMode>(
						(static_cast<UType>(triangle->cull_mode) + max - 1) % max);
				}
				for (TriangleMesh* mesh : active_scene->primitives_factory.GetAllOfType<TriangleMesh>())
				{
					mesh->cull_mode = static_cast<CullMode>(
						(static_cast<UType>(mesh->cull_mode) + max - 1) % max);
				}
				context.logger->LogInfo("Changed/Forwarded cull modes of all triangle-based primitives.");
			}
		}});
		bindings.push_back({ SDL_SCANCODE_F9, "F9: Cycle Filter Mode (Point / Linear / Anisotropic)", [&context]()
		{
			using UType = std::underlying_type_t<FilterMode>;
			context.debug_params.filter_mode = static_cast<FilterMode>(
				(static_cast<UType>(context.debug_params.filter_mode) + 1) % static_cast<UType>(FilterMode::kMax));
			context.logger->LogInfo("Switched to Filter Mode: {}",
				FilterModeToString(context.debug_params.filter_mode));
		}});

		// --- Shared Utility ---
		bindings.push_back({ SDL_SCANCODE_F10, "F10: Toggle Scene Updating", [&context]()
		{
			context.debug_params.update_active_scene = !context.debug_params.update_active_scene;
			context.logger->LogInfo("Scene Updating: {}", context.debug_params.update_active_scene);
		}});
		bindings.push_back({ SDL_SCANCODE_F11, "F11: Toggle Timer Info (every 2 seconds)", [&context]()
		{
			context.debug_params.print_timer = !context.debug_params.print_timer;
			context.logger->LogInfo("Printing Timer: {}", context.debug_params.print_timer);
		}});
		bindings.push_back({ SDL_SCANCODE_P, "", [&context]()
		{
			Scene* active_scene = context.scene_manager ? context.scene_manager->GetActiveScene() : nullptr;
			if (active_scene)
			{
				const auto& pos = active_scene->camera.GetPosition();
				const auto& fwd = active_scene->camera.GetForward();
				const float fov = active_scene->camera.GetFovAngle();
				const float pitch = std::asin(fwd.y);
				const float yaw = std::atan2(-fwd.x, fwd.z);
				context.logger->LogInfo("=== Camera Settings ===");
				context.logger->LogInfo("camera.SetPosition({{ {:.3f}f, {:.3f}f, {:.3f}f }});", pos.x, pos.y, pos.z);
				context.logger->LogInfo("camera.SetFovAngle({:.1f}f);", fov);
				context.logger->LogInfo("camera.Pitch({:.4f}f);", pitch);
				context.logger->LogInfo("camera.Yaw({:.4f}f);", yaw);
			}
		}});

		return bindings;
	}

	void PrintKeyBindings(const std::vector<KeyBinding>& bindings, const Logger& logger)
	{
		logger.LogInfo("=== Available Keybindings ===");
		for (const auto& binding : bindings)
		{
			if (binding.description != nullptr && binding.description[0] != '\0')
				logger.LogInfo("{}", binding.description);
		}
		logger.LogInfo("=== Camera Controls ===");
		logger.LogInfo("Right Mouse + WASD: Fly around with look");
		logger.LogInfo("Right Mouse + QE: Move up/down");
		logger.LogInfo("Middle Mouse: Pan (move up/down/left/right)");
		logger.LogInfo("Left Mouse: Move forward/backward and turn");
		logger.LogInfo("Shift: Speed boost for movement");
		logger.LogInfo("===============================");
	}
}
