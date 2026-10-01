//==============================================================
//  Graphics Programming 1 (2026-2027)
//  Authors : Matthieu Delaere
//  Copyright (c) 2026 Matthieu Delaere. All rights reserved.
//==============================================================
//--- Renderer Selection ---
#define SOFTWARE_PATH_TRACER
//#define SOFTWARE_RASTERIZER
//#define HARDWARE_RASTERIZER

//--- Defines ---
#if defined(HARDWARE_RASTERIZER)
	#define USE_CPU_SURFACE 0
#else
	#define USE_CPU_SURFACE 1
#endif

//--- Standard Includes ---
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <filesystem>

//--- Framework Includes ---
#include <context.h>
#include <scenes.h>
#include <renderer.h>
#if defined(SOFTWARE_PATH_TRACER)
	#include <software_path_tracer.h>
#elif defined(SOFTWARE_RASTERIZER)
	#include <software_rasterizer.h>
#elif defined(HARDWARE_RASTERIZER)
	#include <glad/gl.h>
	#include <hardware_rasterizer.h>
#endif
using namespace gfx;

//--- Frame Limiter ---
#if defined(HARDWARE_RASTERIZER)
	// GPU handles frame pacing (vsync)
	#define CPU_FRAME_LIMITER_ENABLED 0
#else
	#define CPU_FRAME_LIMITER_ENABLED 1
#endif
class FrameLimiter final
{
	double target_ms_{ 0.0 };
	std::chrono::high_resolution_clock::time_point frame_start_ = {};

public:
	FrameLimiter(const double target_fps)
		: target_ms_(1000.0 / target_fps), frame_start_(std::chrono::high_resolution_clock::now())
	{}
	~FrameLimiter()
	{
		const auto frame_end{ std::chrono::high_resolution_clock::now() };
		const double frame_duration_ms{ std::chrono::duration<double, std::milli>(frame_end - frame_start_).count() };

		if (frame_duration_ms < target_ms_)
			SDL_Delay(static_cast<uint32_t>(target_ms_ - frame_duration_ms));
	}
	FrameLimiter(const FrameLimiter&) = delete;
	FrameLimiter& operator=(const FrameLimiter&) = delete;
	FrameLimiter(FrameLimiter&&) = delete;
	FrameLimiter& operator=(FrameLimiter&&) = delete;
};

//--- Helpers ---
namespace
{
	void SaveScreenshot([[maybe_unused]] SDL_Surface* surface,
		[[maybe_unused]] SDL_Window* window, const Logger& logger)
	{
	#if USE_CPU_SURFACE == 1
		if (!surface)
		{
			logger.LogError("[Screenshot] No surface available.");
			return;
		}

		const auto now{ std::chrono::system_clock::now() };
		const auto time{ std::chrono::system_clock::to_time_t(now) };
		std::tm tm_buf{};
	#ifdef _WIN32
		localtime_s(&tm_buf, &time);
	#else
		localtime_r(&time, &tm_buf);
	#endif

		std::ostringstream filename;
		filename << "screenshots/screenshot_"
			<< std::put_time(&tm_buf, "%Y%m%d_%H%M%S") << ".bmp";

		std::filesystem::create_directories("screenshots");

		if (SDL_SaveBMP(surface, filename.str().c_str()))
			logger.LogInfo("[Screenshot] Saved: {}", filename.str());
		else
			logger.LogError("[Screenshot] Failed: {}", SDL_GetError());
	#else
		// Read back the GPU framebuffer via glReadPixels, create an SDL_Surface, and save.
		int w = 0, h = 0;
		SDL_GetWindowSize(window, &w, &h);
		if (w <= 0 || h <= 0)
		{
			logger.LogError("[Screenshot] Invalid window size.");
			return;
		}

		const auto now{ std::chrono::system_clock::now() };
		const auto time{ std::chrono::system_clock::to_time_t(now) };
		std::tm tm_buf{};
	#ifdef _WIN32
		localtime_s(&tm_buf, &time);
	#else
		localtime_r(&time, &tm_buf);
	#endif

		std::ostringstream filename;
		filename << "screenshots/screenshot_"
			<< std::put_time(&tm_buf, "%Y%m%d_%H%M%S") << ".bmp";

		std::filesystem::create_directories("screenshots");

		// Read pixels from the front buffer (RGBA, bottom-to-top).
		std::vector<uint8_t> pixels(static_cast<size_t>(w * h * 4));
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

		// glReadPixels returns rows bottom-to-top; flip vertically for BMP.
		const size_t row_bytes{ static_cast<size_t>(w) * 4 };
		std::vector<uint8_t> row_temp(row_bytes);
		for (int y = 0; y < h / 2; ++y)
		{
			uint8_t* top = pixels.data() + y * row_bytes;
			uint8_t* bot = pixels.data() + (h - 1 - y) * row_bytes;
			std::memcpy(row_temp.data(), top, row_bytes);
			std::memcpy(top, bot, row_bytes);
			std::memcpy(bot, row_temp.data(), row_bytes);
		}

		// Create an SDL_Surface from the pixel data and save.
		SDL_Surface* screenshot = SDL_CreateSurfaceFrom(
			w, h, SDL_PIXELFORMAT_RGBA32, pixels.data(), static_cast<int>(row_bytes));
		if (screenshot)
		{
			if (SDL_SaveBMP(screenshot, filename.str().c_str()))
				logger.LogInfo("[Screenshot] Saved: {}", filename.str());
			else
				logger.LogError("[Screenshot] Failed: {}", SDL_GetError());
			SDL_DestroySurface(screenshot);
		}
		else
		{
			logger.LogError("[Screenshot] Failed to create surface: {}", SDL_GetError());
		}
	#endif
	}

	void SetSurfaceSize(SDL_Window* window, SurfaceInfo& info)
	{
		int current_width{ 0 }, current_height{ 0 };
		SDL_GetWindowSize(window, &current_width, &current_height);
		info.width = static_cast<uint32_t>(current_width);
		info.height = static_cast<uint32_t>(current_height);
	}
}

//--- Main ---
int main(int, char**)
{
	// Specify settings.
	constexpr uint32_t width{ 640 };
	constexpr uint32_t height{ 480 };
	const char* window_title{ "Thibe Provost" };

	// Create context and construct objects.
	Context context{};
	context.logger = std::make_unique<Logger>(LoggerType::kConsole | LoggerType::kFile);
	context.leak_detector = std::make_unique<LeakDetector>(
		context.logger->IsLoggerTypeEnabled(LoggerType::kFile) ? context.logger->GetReportFilename() : "");
	context.timer = std::make_unique<Timer>();
	context.scene_manager = std::make_unique<SceneManager>();

	// Specify ID of leaks here (if any).
	//LeakDetector::BreakOnAllocationId(id);

	// Create SDL window and acquire surface (if enabled).
	#ifdef GFX_PLATFORM_LINUX // VM issues with broken Wayland support, force X11 for now.
		SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
	#endif
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		context.logger->LogCritical("SDL_Init failed: {}", SDL_GetError());
		return 1;
	}
	std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> window = { nullptr, SDL_DestroyWindow };
	window.reset(SDL_CreateWindow(window_title, width, height, 0));
	if (!window)
	{
		context.logger->LogCritical("SDL_CreateWindow failed: {}", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Set options - disable resizing for now.
	SDL_SetWindowResizable(window.get(), false);

	// Get the surface information. This will be either a handle or pixel buffer.
	SDL_Surface* surface = nullptr;
	#if USE_CPU_SURFACE == 1
	{
		surface = SDL_GetWindowSurface(window.get());
		context.surface_info.pixel_format_details = SDL_GetPixelFormatDetails(surface->format);
		context.surface_info.pixel_buffer = static_cast<uint32_t*>(surface->pixels);
	}
	#else
	{
		(void)surface;
		SDL_PropertiesID props = SDL_GetWindowProperties(window.get());
	#if defined(GFX_PLATFORM_WINDOWS)
		context.surface_info.window_os_handle =
			SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	#elif defined(GFX_PLATFORM_WAYLAND)
		context.surface_info.window_os_handle =
			SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
		context.surface_info.display_os_handle =
			SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
	#elif defined(GFX_PLATFORM_X11)
		context.surface_info.window_os_handle =
			reinterpret_cast<void*>(static_cast<uintptr_t>(
				SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
		context.surface_info.display_os_handle =
			SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
	#endif
		// Set initial dimensions for the GPU renderer.
		context.surface_info.width = width;
		context.surface_info.height = height;
	}
	#endif

	// Create scenes based on active renderer.
	#if defined(SOFTWARE_PATH_TRACER)
		context.scene_manager->CreateScene<BasicScene>();
	#elif defined(SOFTWARE_RASTERIZER)
	#elif defined(HARDWARE_RASTERIZER)
	#endif

	// Create renderer based on active define.
	std::unique_ptr<Renderer> renderer = nullptr;
	#if defined(SOFTWARE_PATH_TRACER)
		renderer = std::make_unique<SoftwarePathTracer>(&context);
	#elif defined(SOFTWARE_RASTERIZER)
		renderer = std::make_unique<SoftwareRasterizer>(&context);
	#elif defined(HARDWARE_RASTERIZER)
		renderer = std::make_unique<HardwareRasterizer>(&context);
	#endif

	// Register key bindings and add screenshot key binding.
	std::vector<KeyBinding> key_bindings = RegisterKeyBindings(context);
	key_bindings.push_back({ SDL_SCANCODE_F12, "F12: Save Screenshot",
		[&surface, &window, &context]()
	{
		SaveScreenshot(surface, window.get(), *context.logger);
	}});

	// Print info.
	PrintKeyBindings(key_bindings, *context.logger);

	// Tick once before the loop to reset the baseline, avoiding a large initial delta time spike.
	context.timer->Tick();
	bool is_looping{ true };
	while (is_looping)
	{
		// Limit the entire frame if enabled.
		#if CPU_FRAME_LIMITER_ENABLED
			FrameLimiter limiter(120.0);
		#endif

		// Tick the timer at the start of the frame so all systems use the same delta time.
		context.timer->Tick();

		// Reset per-frame flags before input and update.
		context.scene_manager->BeginFrame();

		// Get active scene, if any.
		Scene* active_scene = context.scene_manager->GetActiveScene();

		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_EVENT_QUIT:
				is_looping = false;
				break;
			case SDL_EVENT_KEY_UP:
				for (const auto& binding : key_bindings)
				{
					if (e.key.scancode == binding.key)
					{
						binding.action();
						break;
					}
				}
				break;
			default:
				break;
			}
		}

		//----------- UE5-style camera controls -----------
		if (active_scene)
		{
			const float delta_time{ static_cast<float>(context.timer->GetDeltaTime()) / 1000.f };
			constexpr float mouse_sensitivity{ 0.003f };
			constexpr float movement_speed{ 3.0f };
			constexpr float rotation_speed{ 1.0f };
			constexpr float speed_boost_multiplier{ 3.0f };

			// Get input states.
			const bool* keyboard_state = SDL_GetKeyboardState(nullptr);
			float mouse_x{ 0.f }, mouse_y{ 0.f };
			const SDL_MouseButtonFlags mouse_state = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

			// Calculate current speeds (with speed boost if shift is held).
			const bool speed_boost{ keyboard_state[SDL_SCANCODE_LSHIFT] || keyboard_state[SDL_SCANCODE_RSHIFT] };
			const float current_movement_speed{ movement_speed * (speed_boost ? speed_boost_multiplier : 1.0f) * delta_time };
			constexpr float current_rotation_speed{ rotation_speed * mouse_sensitivity };

			// Right Mouse Button: Look around (pitch/yaw).
			if (mouse_state & SDL_BUTTON_RMASK)
			{
				active_scene->camera.Pitch(-mouse_y * current_rotation_speed);
				active_scene->camera.Yaw(mouse_x * current_rotation_speed);

				// Right Mouse + WASD: Fly around with look.
				if (keyboard_state[SDL_SCANCODE_W])
					active_scene->camera.MoveForward(current_movement_speed);
				if (keyboard_state[SDL_SCANCODE_S])
					active_scene->camera.MoveForward(-current_movement_speed);
				if (keyboard_state[SDL_SCANCODE_A])
					active_scene->camera.MoveRight(-current_movement_speed);
				if (keyboard_state[SDL_SCANCODE_D])
					active_scene->camera.MoveRight(current_movement_speed);
				if (keyboard_state[SDL_SCANCODE_Q])
					active_scene->camera.MoveUp(-current_movement_speed);
				if (keyboard_state[SDL_SCANCODE_E])
					active_scene->camera.MoveUp(current_movement_speed);
			}
			// Middle Mouse Button: Pan (move up/down/left/right).
			else if (mouse_state & SDL_BUTTON_MMASK)
			{
				active_scene->camera.MoveRight(-mouse_x * current_movement_speed * 0.1f);
				active_scene->camera.MoveUp(mouse_y * current_movement_speed * 0.1f);
			}
			// Left Mouse: Move forward/backward and turn
			else if (mouse_state & SDL_BUTTON_LMASK)
			{
				active_scene->camera.MoveForward(-mouse_y * current_movement_speed * 0.1f);
				active_scene->camera.Yaw(mouse_x * current_rotation_speed);
			}
		}

		//--------- Update ---------
		if (context.debug_params.print_timer)
			context.timer->PrintInformation(2.0, context.logger.get());
		context.scene_manager->UpdateActiveScene(
			context.timer->GetDeltaTime() / 1000.0,
			context.debug_params.update_active_scene);

		//--------- Render ---------
		// Update surface dimensions each frame (handles window resize if enabled later).
		SetSurfaceSize(window.get(), context.surface_info);
		renderer->Render();

		// Update window title with stats.
		{
			const double dt_ms{ context.timer->GetDeltaTime() };
			const double fps{ (dt_ms > 0.0) ? 1000.0 / dt_ms : 0.0 };
			std::string title = std::format("{} | FPS: {:.0f} | {:.1f}ms", window_title, fps, dt_ms);
			if (active_scene)
				title += std::format(" | Prim: {} | Tris: {}", active_scene->GetPrimitiveCount(), active_scene->GetTriangleCount());
		#if defined(SOFTWARE_PATH_TRACER)
			title += std::format(" | Samples: {}", context.debug_params.accumulated_samples);
		#endif
			SDL_SetWindowTitle(window.get(), title.c_str());
		}
		#if USE_CPU_SURFACE == 1
		{
			SDL_UpdateWindowSurface(window.get());
		}
		#endif
	}

	renderer.reset();
	window.reset();
	SDL_Quit();
	return 0;
}
