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
	allow_present_ = true;

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

	engine_.signal_block();
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
#if 1
	engine_.signal([&] {
		if (!allow_present_)
			return;
		float g[4] = {.5f, .5f, 1.f, 1.f};
		d3d_immediate_context_->ClearRenderTargetView(d3d_render_target_.get(), g);
		//ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->Present(DXGI_SWAP_EFFECT_DISCARD, 0));
		dxgi_swap_chain_->Present(DXGI_SWAP_EFFECT_DISCARD, 0);
	});
#endif
}

auto context_t::signal_fullscreen_toggle(uint32_t output_index) -> void
{
	engine_.signal([&, output_index]
	{
		ATMA_ASSERT(dxgi_adapter_);

		fullscreen_ = !fullscreen_;
		window_->fullscreen_ = fullscreen_;

		allow_present_ = false;

		if (fullscreen_)
		{
			// get output of our adapter
			dxgi_output_ = voodoo::output_at(dxgi_adapter_, output_index);
			ATMA_ASSERT(dxgi_output_);

			// find best-fitting fullscreen resolution
			DXGI_OUTPUT_DESC output_desc;
			dxgi_output_->GetDesc(&output_desc);

			auto candidate = DXGI_MODE_DESC{800, 600, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED};
			auto mode = DXGI_MODE_DESC{};
			dxgi_output_->FindClosestMatchingMode(&candidate, &mode, d3d_device_.get());
			//std::cout << "mode found: " << mode.Width << "x" << mode.Height << std::endl;

			dxgi_swap_chain_->ResizeTarget(&mode);

			dxgi_swap_chain_->SetFullscreenState(true, dxgi_output_.get());
		}
		else
		{
			dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);

			// resize window back to what it was before fullscreening
			auto mode = DXGI_MODE_DESC{display_format_.width, display_format_.height, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED};
			dxgi_swap_chain_->ResizeTarget(&mode);
		}

		allow_present_ = true;
	});
}

auto context_t::signal_create_swapchain() -> void
{
	display_format_.width = window_->width();
	display_format_.height = window_->height();

	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{0, 0, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT, 1, window_->hwnd(), TRUE,
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
	if (!fullscreen_) {
		display_format_.width = e.width();
		display_format_.height = e.height();
	}

	//engine_.signal([&, e] {
		if (e.origin().expired())
			return;

		auto wnd = std::dynamic_pointer_cast<fooey::window_t>(e.origin().lock());
		ATMA_ASSERT(wnd);

		std::cout << "on_resize " << e.width() << "x" << e.height() << std::endl;

		// teardown backbuffer rendertarget
		d3d_render_target_.reset();

		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

		
		// rebind backbuffer again
		atma::com_ptr<ID3D11Texture2D> backbuffer;
		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.assign()));
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateRenderTargetView(backbuffer.get(), nullptr, d3d_render_target_.assign()));
		d3d_immediate_context_->OMSetRenderTargets(1, &d3d_render_target_.get_ref(), nullptr);

		std::cout << "on_resize - done" << std::endl;
	//});
}

auto context_t::create_d3d_buffer(voodoo::d3d_buffer_ptr& buffer, gpu_access_t gpu_access, cpu_access_t cpu_access, uint32_t data_size, void* data) -> void
{
	// calcualte the buffer usage based off our gpu-access/cpu-access flags
	D3D11_USAGE buffer_usage = D3D11_USAGE_DEFAULT;
	if (cpu_access == cpu_access_t::write) {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_DYNAMIC;
		}
		else if (gpu_access == gpu_access_t::write || gpu_access == gpu_access_t::read_write) {
			buffer_usage = D3D11_USAGE_STAGING;
		}
	}
	else if (cpu_access == cpu_access_t::read || cpu_access == cpu_access_t::read_write) {
		buffer_usage = D3D11_USAGE_STAGING;
	}
	else {
		if (gpu_access == gpu_access_t::read) {
			buffer_usage = D3D11_USAGE_IMMUTABLE;
		}
	}

	// cpu-usage still needs to be calculated
	D3D11_CPU_ACCESS_FLAG cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(0L);
	switch (cpu_access) {
		case cpu_access_t::read: cpua = D3D11_CPU_ACCESS_READ; break;
		case cpu_access_t::write: cpua = D3D11_CPU_ACCESS_WRITE; break;
		case cpu_access_t::read_write: cpua = static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE); break;
	}

	D3D11_BUFFER_DESC buffer_desc{data_size, buffer_usage, D3D11_BIND_VERTEX_BUFFER, cpua, 0, 0};


	if (data) {
		// for vertex buffers, the pitch and slice-pitch mean nothing, so we'll just
		// pass along stats to help with debugging (1xdata_size buffer created)
		D3D11_SUBRESOURCE_DATA d3d_data{data, 1, data_size};
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateBuffer(&buffer_desc, &d3d_data, buffer.assign()));
	}
	else {
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateBuffer(&buffer_desc, NULL, buffer.assign()));
	}
}


//auto context_t::immediate_d3d_map(voodoo::d3d_buffer_ptr& buffer, D3D11_MAPPED_SUBRESOURCE* mapped_resource, D3D11_MAP map_type, uint32_t subresource) -> void
//{
//	d3d_deferred_context_->Map(buffer.get(), subresource, map_type, 0, mapped_resource);
//}

//auto context_t::signal_d3d_map(voodoo::d3d_buffer_ptr& buffer, D3D11_MAPPED_SUBRESOURCE* mapped_resource, D3D11_MAP map_type, uint32_t subresource) -> void
//{
//	engine_.signal([&, mapped_resource, map_type, subresource] {
//		d3d_immediate_context_->Map(buffer.get(), subresource, map_type, 0, mapped_resource);
//	});
//}

auto context_t::signal_d3d_map(voodoo::d3d_buffer_ptr& buffer, D3D11_MAPPED_SUBRESOURCE* mapped_resource, D3D11_MAP map_type, uint32_t subresource, std::function<void(D3D11_MAPPED_SUBRESOURCE*)> const& fn) -> void
{
	engine_.signal([&, mapped_resource, map_type, subresource] {
		d3d_immediate_context_->Map(buffer.get(), subresource, map_type, 0, mapped_resource);
		if (fn)
			fn(mapped_resource);
	});
}

auto context_t::signal_d3d_unmap(voodoo::d3d_buffer_ptr& buffer, uint32_t subresource) -> void
{
	engine_.signal([&, buffer, subresource] {
		d3d_immediate_context_->Unmap(buffer.get(), subresource);
	});
}


