#ifndef SHINY_CONTEXT_HPP
#define SHINY_CONTEXT_HPP
//======================================================================
#include <shiny/voodoo/device.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
//======================================================================
#include <atma/intrusive_ptr.hpp>
//======================================================================
#include <thread>
#include <atomic>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
//======================================================================
		
	enum class display_format_t
	{
		unknown,
		r32g32b32a32 = 1,
		r32g32b32a32_f32 = 2,
		r8g8b8a8_unorm = 28
	};

	struct display_mode_t
	{
		uint32_t width, height;
		uint32_t refreshrate_frames, refreshrate_period;
		display_format_t format;
	};

	struct defer_construction_t {};
	static defer_construction_t const defer_construction;

	struct context_t : std::enable_shared_from_this<context_t>
	{
		//context_t(bool fullscreen, uint32_t width, uint32_t height);
		context_t(defer_construction_t);
		~context_t();

		auto bind_to(fooey::window_ptr const&) -> void;
		auto toggle_fullscreen() -> void;

	private:
		auto enumerate_backbuffers() -> void;
		auto create_swapchain() -> void; 
		
		auto closest_fullscreen_backbuffer_mode(uint32_t width, uint32_t height)->display_mode_t;
		
		// dxgi
		IDXGIDevice1* dxgi_device_;
		IDXGIAdapter1* dxgi_adapter_;
		IDXGIFactory1* dxgi_factory_;
		IDXGISwapChain* dxgi_swap_chain_;

		// fooey
		fooey::window_ptr window_;
		//atma::event_t<uint32_t, uint32_t>::delegate_handle_t on_resize_handle_;

		// implementation
		typedef std::vector<display_mode_t> display_modes_t;
		display_modes_t backbuffer_display_modes_;
		uint32_t width_, height_;
		bool fullscreen_;
	};
	//typedef atma::intrusive_ptr<context_t> context_ptr;
	typedef std::shared_ptr<context_t> context_ptr;
	typedef std::weak_ptr<context_t> context_wptr;
	
	auto create_context(defer_construction_t) -> context_ptr;

	auto signal_fullscreen_toggle(context_ptr const& context) -> void;

//======================================================================
} // namespace shiny
//======================================================================
#endif
//======================================================================
