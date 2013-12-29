#include <shiny/context.hpp>

#include <shiny/voodoo/prime_thread.hpp>
#include <shiny/voodoo/device.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>

#include <vector>

using namespace shiny;
using shiny::context_t;

namespace
{
	struct monitor_info_t
	{
		int32_t width, height;
	};

	auto primary_monitor_resolution() -> monitor_info_t
	{
		return { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	}
}

//======================================================================
// context creation
//======================================================================
auto shiny::create_context(fooey::window_ptr const& window, uint32_t adapter) -> shiny::context_ptr
{
	return context_ptr(new context_t(window, adapter));
}

//======================================================================
// context_t
//======================================================================
context_t::context_t(fooey::window_ptr const& window, uint32_t adapter)
: fullscreen_()
{
	setup_dxgi_and_d3d(adapter);
	bind_to(window);
	bind_events(window);
}

context_t::~context_t()
{
	if (window_)
		window_->unbind(bound_events_);
}

auto context_t::setup_dxgi_and_d3d(uint32_t adapter) -> void
{
	std::tie(dxgi_adapter_, d3d_device_, d3d_immediate_context_)
		= voodoo::dxgi_and_d3d_at(adapter);
}


auto context_t::bind_to(fooey::window_ptr const& window) -> void
{
	window_ = window;
	//width_ = window_->width_in_pixels();
	//height_ = window_->height_in_pixels();

	create_swapchain();
}

auto context_t::bind_events(fooey::window_ptr const& window) -> void
{
	bound_events_ = window->on({
		{"resize-dc.shiny.context", [this](fooey::events::resize_t& e) { on_resize(e); }}
	});
}

auto context_t::on_resize(fooey::events::resize_t& e) -> void
{
	voodoo::prime_thread::enqueue([this, e]
	{
		if (e.origin().expired())
			return;

		auto wnd = std::dynamic_pointer_cast<fooey::window_t>(e.origin().lock());
		ATMA_ASSERT(wnd);

		if (fullscreen_)
		{
			//fullscreen_width_ = e.width();
			//fullscreen_height_ = e.height();

			ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(3, e.width(), e.height(), DXGI_FORMAT_UNKNOWN, 0));
		}
		else
		{
			//width_ = e.width();
			//height_ = e.height();
			ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
		}
	});
}

auto context_t::create_swapchain() -> void
{
	display_format_.width = window_->width();
	display_format_.height = window_->height();

	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{0, 0, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{1, 0},
		0, 3, window_->hwnd(), TRUE, DXGI_SWAP_EFFECT_DISCARD, 0
	};

#if 1
	//ATMA_ENSURE_IS(S_OK, detail::dxgi_factory_->CreateSwapChain(&d3d_device_.assign(), &desc, &dxgi_swap_chain_));
	//ATMA_ENSURE_IS(S_OK, detail::dxgi_factory_->MakeWindowAssociation(window_->hwnd(), DXGI_MWA_NO_WINDOW_CHANGES));
#endif
}



auto context_t::toggle_fullscreen() -> void
{
#if 0
	fullscreen_ = !fullscreen_;
	window_->fullscreen_ = fullscreen_;

	if (fullscreen_)
	{
		auto monitor_info = primary_monitor_resolution();

		// find best-fitting fullscreen resolution
		DXGI_OUTPUT_DESC output_desc;
		detail::dxgi_primary_output_->GetDesc(&output_desc);
		
		auto candidate = DXGI_MODE_DESC{800, 640, {0, 0}, DXGI_FORMAT_UNKNOWN, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED};
		auto mode_desc = DXGI_MODE_DESC{};
		detail::dxgi_primary_output_->FindClosestMatchingMode(&candidate, &mode_desc, detail::d3d_device_.get()/*detail::dxgi_device_.get()*/);


		// resize-target to natural width/height
		dxgi_swap_chain_->ResizeTarget(&mode_desc);


		// go to fullscreen
		dxgi_swap_chain_->SetFullscreenState(true, detail::dxgi_primary_output_.get());




		// second resize-target with zeroed refresh-rate because MS says so
		mode_desc.RefreshRate = {0, 0};
		dxgi_swap_chain_->ResizeTarget(&mode_desc);
	}
	else
	{
		dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);

		//SetWindowPos(window_->hwnd, NULL, win)
		//window_->resize()
		//fooey::signal_window_resize(window_, )
		//fooey::signal_window_resize(window_);
	}
#endif
}

#if 0
auto context_t::closest_fullscreen_backbuffer_mode(uint32_t width, uint32_t height) -> shiny::display_mode_t
{
	for (auto const& x : backbuffer_display_modes_)
	{
		if (x.width >= width && x.height >= height)
			return x;
	}

	return backbuffer_display_modes_.back();
}
#endif

auto shiny::signal_fullscreen_toggle(context_ptr const& context) -> void
{
	voodoo::prime_thread::enqueue(std::bind(&context_t::toggle_fullscreen, context));
}
