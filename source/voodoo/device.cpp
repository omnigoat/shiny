#include <shiny/voodoo/device.hpp>
#include <atma/assert.hpp>

//======================================================================
// externs
//======================================================================
ID3D11Device* shiny::voodoo::detail::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::voodoo::detail::d3d_immediate_context_ = nullptr;
std::mutex shiny::voodoo::detail::immediate_context_mutex_;

__declspec(thread) ID3D11DeviceContext* shiny::voodoo::detail::d3d_local_context_ = nullptr;


//======================================================================
// scoped_IC_lock
//======================================================================
using shiny::voodoo::detail::scoped_async_immediate_context_t;
scoped_async_immediate_context_t::scoped_async_immediate_context_t()
{
	immediate_context_mutex_.lock();
}

scoped_async_immediate_context_t::~scoped_async_immediate_context_t()
{
	immediate_context_mutex_.unlock();
}

auto scoped_async_immediate_context_t::operator -> () const -> ID3D11DeviceContext* {
	return d3d_immediate_context_;
}


//======================================================================
// device management
//======================================================================
auto shiny::voodoo::setup_d3d_device() -> void
{
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&detail::d3d_device_,
		NULL,
		&detail::d3d_immediate_context_
	));

	detail::d3d_local_context_ = detail::d3d_immediate_context_;
}

auto shiny::voodoo::teardown_d3d_device() -> void
{
	detail::d3d_immediate_context_->Release();
	detail::d3d_device_->Release();
}

//======================================================================
// context creation
//======================================================================
auto shiny::voodoo::create_context(fooey::window_ptr const& window, uint32_t width, uint32_t height) -> shiny::voodoo::context_ptr
{
	return context_ptr(new context_t(window, width, height));
}

using shiny::voodoo::context_t;
context_t::context_t(fooey::window_ptr const& window, uint32_t width, uint32_t height)
	: window_(window), width_(width), height_(height), fullscreen_()
{
	if (width_ == 0)
		width_ = window_->width_in_pixels();
	if (height_ == 0)
		height_ = window_->height_in_pixels();

	IDXGIDevice1* dxgi_device = nullptr;
	ATMA_ENSURE_IS(S_OK, detail::d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device));

	IDXGIAdapter1* dxgi_adapter = nullptr;
	ATMA_ENSURE_IS(S_OK, dxgi_device->GetParent(__uuidof(IDXGIAdapter1), (void**)&dxgi_adapter));

	IDXGIFactory1* dxgi_factory = nullptr;
	ATMA_ENSURE_IS(S_OK, dxgi_adapter->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgi_factory));

	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{width_, height_, { 0, 0 }, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{ 1, 0 },
		0,
		3,
		window->hwnd,
		TRUE,
		DXGI_SWAP_EFFECT_DISCARD,
		0
	};

	IDXGISwapChain* dxgi_swap_chain = nullptr;
	ATMA_ENSURE_IS(S_OK, dxgi_factory->CreateSwapChain(detail::d3d_device_, &desc, &dxgi_swap_chain));


	on_resize_handle_ = window_->on_resize.connect([dxgi_swap_chain](atma::event_flow_t&, uint32_t width, uint32_t height) {
		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));
	});
}

context_t::~context_t()
{
	window_->on_resize.disconnect(on_resize_handle_);
}

auto context_t::toggle_fullscreen() -> void
{
	//dxgi_swap_chain_->SetFullscreenState(TRUE, wat);
	//dxgi_swap_chain_->ResizeTarget()
}
