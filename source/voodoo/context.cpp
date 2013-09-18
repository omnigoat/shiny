#include <shiny/voodoo/context.hpp>
#include <shiny/voodoo/prime_thread.hpp>

//======================================================================
// context creation
//======================================================================
auto shiny::create_context(fooey::window_ptr const& window, uint32_t width, uint32_t height) -> shiny::context_ptr
{
	return context_ptr(new context_t(window, width, height));
}

using shiny::context_t;
context_t::context_t(fooey::window_ptr const& window, uint32_t width, uint32_t height)
	: window_(window), width_(width), height_(height), fullscreen_()
{
	if (width_ == 0)
		width_ = window_->width_in_pixels();
	if (height_ == 0)
		height_ = window_->height_in_pixels();

	ATMA_ENSURE_IS(S_OK, voodoo::detail::d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device_));
	ATMA_ENSURE_IS(S_OK, dxgi_device_->GetParent(__uuidof(IDXGIAdapter1), (void**)&dxgi_adapter_));
	ATMA_ENSURE_IS(S_OK, dxgi_adapter_->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgi_factory_));

	enumerate_backbuffers();
	create_swapchain();

	auto cptr = shared_from_this();
	on_resize_handle_ = window_->on_resize.connect([cptr](atma::event_flow_t& fc, uint32_t width, uint32_t height)
	{
		voodoo::prime_thread::enqueue([&cptr, &fc, width, height]
		{
			if (!cptr->fullscreen_)
			{
				cptr->width_ = width;
				cptr->height_ = height;
				std::cout << "resizing to " << cptr->width_ << "x" << cptr->height_ << std::endl;
				ATMA_ENSURE_IS(S_OK, cptr->dxgi_swap_chain_->ResizeBuffers(3, cptr->width_, cptr->height_, DXGI_FORMAT_UNKNOWN, 0));
			}
		});
	});
}

context_t::~context_t()
{
	window_->on_resize.disconnect(on_resize_handle_);
}

auto context_t::enumerate_backbuffers() -> void
{
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

	IDXGIOutput* out = nullptr;
	ATMA_ENSURE_IS(S_OK, dxgi_adapter_->EnumOutputs(1, &out));

	uint32_t mode_count = 0;
	ATMA_ENSURE_IS(S_OK, out->GetDisplayModeList(format, 0, &mode_count, nullptr));
	
	auto modes = std::unique_ptr<DXGI_MODE_DESC[]>(new DXGI_MODE_DESC[mode_count]);
	ATMA_ENSURE_IS(S_OK, out->GetDisplayModeList(format, 0, &mode_count, modes.get()));
	
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
		0, 3, window_->hwnd, TRUE, DXGI_SWAP_EFFECT_DISCARD, 0
	};

	ATMA_ENSURE_IS(S_OK, dxgi_factory_->CreateSwapChain(voodoo::detail::d3d_device_, &desc, &dxgi_swap_chain_));
}



auto context_t::toggle_fullscreen() -> void
{
	fullscreen_ = !fullscreen_;

	if (fullscreen_)
	{
		auto mode = closest_fullscreen_backbuffer_mode(width_, height_);
		auto dxgi_mode = DXGI_MODE_DESC{mode.width, mode.height, {mode.refreshrate_frames, mode.refreshrate_period}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED};
		
		std::cout << "SHINY: going fullscreen to " << mode.width << "x" << mode.height << std::endl;
		dxgi_swap_chain_->ResizeTarget(&dxgi_mode);
		dxgi_swap_chain_->SetFullscreenState(TRUE, nullptr);
	}
	else
	{
		std::cout << "SHINY: going windowed to " << width_ << "x" << height_ << std::endl;
		auto dxgi_mode = DXGI_MODE_DESC{width_, height_, {0, 1}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED};
		dxgi_swap_chain_->ResizeTarget(&dxgi_mode);
		dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);

		//fooey::signal_window_resize(window_, width_, height_);
		
		RECT rect;
		GetWindowRect(window_->hwnd, &rect);
		SetWindowPos(window_->hwnd, HWND_TOPMOST, rect.left, rect.top, window_->width_in_pixels(), window_->height_in_pixels(), 0);
	}
}

auto context_t::closest_fullscreen_backbuffer_mode(uint32_t width, uint32_t height) -> shiny::display_mode_t
{
	for (auto const& x : backbuffer_display_modes_)
	{
		if (x.width > width && x.height > height)
			return x;
	}

	return backbuffer_display_modes_.back();
}

auto shiny::signal_fullscreen_toggle(context_ptr const& context) -> void
{
	voodoo::prime_thread::enqueue(std::bind(&context_t::toggle_fullscreen, context));
}
