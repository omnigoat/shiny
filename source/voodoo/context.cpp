#include <shiny/voodoo/context.hpp>
#include <shiny/voodoo/prime_thread.hpp>
#include <shiny/voodoo/device.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>


using namespace shiny::voodoo;

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
auto shiny::create_context(fooey::window_ptr const& window) -> shiny::context_ptr
{
	return context_ptr(new context_t(window));
}

auto shiny::create_context(shiny::defer_construction_t) -> shiny::context_ptr
{
	return context_ptr(new context_t(shiny::defer_construction));
}


//======================================================================
// context_t
//======================================================================
using shiny::context_t;

context_t::context_t(fooey::window_ptr const& window)
	: context_t(shiny::defer_construction_t())
{
	bind_to(window);
	bind_events(window);
}

context_t::context_t(shiny::defer_construction_t)
	: width_(), height_(), fullscreen_()
{
	enumerate_backbuffers();
}


context_t::~context_t()
{
	if (window_)
		window_->unbind(bound_events_);
}


auto context_t::bind_to(fooey::window_ptr const& window) -> void
{
	window_ = window;
	width_ = window_->width_in_pixels();
	height_ = window_->height_in_pixels();

	create_swapchain();
	
	auto ctx = this;

	
}

auto context_t::bind_events(fooey::window_ptr const& window) -> void
{
	auto ctx = this;

	bound_events_ = window->on({
		{"resize-dc.shiny.context", [ctx](fooey::events::resize_t& e) {
			voodoo::prime_thread::enqueue([ctx, e]
			{
				if (e.origin().expired())
					return;

				auto wnd = std::dynamic_pointer_cast<fooey::window_t>(e.origin().lock());
				ATMA_ASSERT(wnd);

				std::cout << "resize-dc.shiny.context: resizing to " << e.width() << "x" << e.height() << std::endl;

				if (ctx->fullscreen_)
				{
					ctx->fullscreen_width_ = e.width();
					ctx->fullscreen_height_ = e.height();

					ATMA_ENSURE_IS(S_OK, ctx->dxgi_swap_chain_->ResizeBuffers(3, ctx->fullscreen_width_, ctx->fullscreen_height_, DXGI_FORMAT_UNKNOWN, 0));
				}
				else
				{
					ctx->width_ = e.width();
					ctx->height_ = e.height();
					ATMA_ENSURE_IS(S_OK, ctx->dxgi_swap_chain_->ResizeBuffers(3, ctx->width_, ctx->height_, DXGI_FORMAT_UNKNOWN, 0));
				}
			});
		}}
	});
}

auto context_t::enumerate_backbuffers() -> void
{
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

	uint32_t mode_count = 0;
	ATMA_ENSURE_IS(S_OK, detail::dxgi_primary_output_->GetDisplayModeList(format, 0, &mode_count, nullptr));
	
	auto modes = std::unique_ptr<DXGI_MODE_DESC[]>(new DXGI_MODE_DESC[mode_count]);
	ATMA_ENSURE_IS(S_OK, detail::dxgi_primary_output_->GetDisplayModeList(format, 0, &mode_count, modes.get()));
	
	// convert dxgi format to shiny's format
	for (auto i = modes.get(); i != modes.get() + mode_count; ++i)
	{
		backbuffer_display_modes_.push_back({
			i->Width, i->Height,
			i->RefreshRate.Numerator, i->RefreshRate.Denominator,
			display_format_t::r8g8b8a8_unorm
		});
	}
}

auto context_t::create_swapchain() -> void
{
	auto format = closest_fullscreen_backbuffer_mode(width_, height_);
	
	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{format.width, format.height, {format.refreshrate_frames, format.refreshrate_period}, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{1, 0},
		0, 3, window_->hwnd(), TRUE, DXGI_SWAP_EFFECT_DISCARD, 0
	};

	ATMA_ENSURE_IS(S_OK, detail::dxgi_factory_->CreateSwapChain(voodoo::detail::d3d_device_.get(), &desc, &dxgi_swap_chain_));
	ATMA_ENSURE_IS(S_OK, detail::dxgi_factory_->MakeWindowAssociation(window_->hwnd(), DXGI_MWA_NO_WINDOW_CHANGES));
}



auto context_t::toggle_fullscreen() -> void
{
	fullscreen_ = !fullscreen_;

	if (fullscreen_)
	{
		auto monitor_info = primary_monitor_resolution();

		// find best-fitting fullscreen resolution
		DXGI_OUTPUT_DESC output_desc;
		detail::dxgi_primary_output_->GetDesc(&output_desc);
		
		DXGI_MODE_DESC candidate{output_desc.DesktopCoordinates.right, output_desc.DesktopCoordinates.bottom, {0, 0}, DXGI_FORMAT_UNKNOWN, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED};
		DXGI_MODE_DESC mode_desc;
		detail::dxgi_primary_output_->FindClosestMatchingMode(&candidate, &mode_desc, detail::dxgi_device_.get());

		// resize-target to natural width/height
		dxgi_swap_chain_->ResizeTarget(&mode_desc);

		// go to fullscreen
		dxgi_swap_chain_->SetFullscreenState(TRUE, nullptr);

		// second resize-target with zeroed refresh-rate because MS says so
		mode_desc.RefreshRate ={0, 0};
		dxgi_swap_chain_->ResizeTarget(&mode_desc);
	}
	else
	{
		dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);
	}
}

auto context_t::closest_fullscreen_backbuffer_mode(uint32_t width, uint32_t height) -> shiny::display_mode_t
{
	for (auto const& x : backbuffer_display_modes_)
	{
		if (x.width >= width && x.height >= height)
			return x;
	}

	return backbuffer_display_modes_.back();
}

auto shiny::signal_fullscreen_toggle(context_ptr const& context) -> void
{
	voodoo::prime_thread::enqueue(std::bind(&context_t::toggle_fullscreen, context));
}
