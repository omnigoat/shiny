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
	engine_.signal([=]{
		std::tie(dxgi_adapter_, d3d_device_, d3d_immediate_context_) = voodoo::dxgi_and_d3d_at(adapter);
		window_ = window;
		signal_create_swapchain();
		bind_events(window);
	});
	signal_setup_backbuffer();
	signal_block();
}

context_t::~context_t()
{
	engine_.signal([&]{
		if (window_)
			window_->unbind(bound_events_);
	});
}


auto context_t::bind_events(fooey::window_ptr const& window) -> void
{
	bound_events_ = window->on({
		{"resize-dc.shiny.context", [this](fooey::events::resize_t& e) { on_resize(e); }}
	});
}




auto context_t::signal_block() -> void
{
	engine_.signal_block();
}


auto context_t::signal_present() -> void
{
	engine_.signal([&] {
		float g[4] = {.5f, .5f, 1.f, 1.f};
		d3d_immediate_context_->ClearRenderTargetView(d3d_render_target_.get(), g);
		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->Present(DXGI_SWAP_EFFECT_DISCARD, 0));
	});
}

auto context_t::signal_fullscreen_toggle(uint32_t output_index) -> void
{
	engine_.signal([&, output_index]
	{
		ATMA_ASSERT(dxgi_adapter_);

		fullscreen_ = !fullscreen_;
		window_->fullscreen_ = fullscreen_;

		if (fullscreen_)
		{
			// get output of our adapter
			dxgi_output_ = voodoo::output_at(dxgi_adapter_, output_index);
			ATMA_ASSERT(dxgi_output_);

			// find best-fitting fullscreen resolution
			DXGI_OUTPUT_DESC output_desc;
			dxgi_output_->GetDesc(&output_desc);

			auto fmt = voodoo::closest_matching_format(dxgi_output_, 800, 600);

			auto candidate = DXGI_MODE_DESC{fmt.width, fmt.height, {fmt.refreshrate_frames, fmt.refreshrate_period}, DXGI_FORMAT_UNKNOWN, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED};
			auto mode_desc = DXGI_MODE_DESC{};
			dxgi_output_->FindClosestMatchingMode(&candidate, &mode_desc, d3d_device_.get());



			// when changing modes, ResizeTarget posts WM_SIZE to our window,
			// so we need to wait until the window has processed that message
			dxgi_swap_chain_->ResizeTarget(&mode_desc);
			window_->signal_block();

			// go to fullscreen
			dxgi_swap_chain_->SetFullscreenState(true, dxgi_output_.get());





			// second resize-target with zeroed refresh-rate because MS says so
			mode_desc.RefreshRate = {0, 0};
			dxgi_swap_chain_->ResizeTarget(&mode_desc);
		}
		else
		{
			dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);
		}
	});
}

auto context_t::signal_create_swapchain() -> void
{
	display_format_.width = window_->width();
	display_format_.height = window_->height();

	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{0, 0, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT, 3, window_->hwnd(), TRUE,
		DXGI_SWAP_EFFECT_DISCARD, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	ATMA_ENSURE_IS(S_OK, voodoo::dxgi_factory()->CreateSwapChain(d3d_device_.get(), &desc, dxgi_swap_chain_.assign()));
	ATMA_ENSURE_IS(S_OK, voodoo::dxgi_factory()->MakeWindowAssociation(window_->hwnd(), DXGI_MWA_NO_WINDOW_CHANGES));
}

auto context_t::signal_setup_backbuffer() -> void
{
	engine_.signal([&]{
		// create render-target & set it
		atma::com_ptr<ID3D11Texture2D> backbuffer;
		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.assign()));
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateRenderTargetView(backbuffer.get(), nullptr, d3d_render_target_.assign()));
		d3d_immediate_context_->OMSetRenderTargets(1, &d3d_render_target_.get_ref(), nullptr);
	});
}


auto context_t::on_resize(fooey::events::resize_t& e) -> void
{
	if (e.origin().expired())
		return;

	engine_.signal([&, e] {
		auto wnd = std::dynamic_pointer_cast<fooey::window_t>(e.origin().lock());
		ATMA_ASSERT(wnd);

		// teardown backbuffer rendertarget
		d3d_render_target_.reset();

		if (fullscreen_) {
			ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(3, e.width(), e.height(), DXGI_FORMAT_UNKNOWN, 0));
		}
		else {
			ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
		}
	});

	// rebind backbuffer again
	signal_setup_backbuffer();
}

