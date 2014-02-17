#include <dust/context.hpp>

#include <dust/vertex_declaration.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/runtime.hpp>

#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>

#include <d3dcompiler.h>

#include <vector>
#include <atomic>


using namespace dust;
using dust::context_t;


//======================================================================
// context creation
//======================================================================
auto dust::create_context(runtime_t& runtime, fooey::window_ptr const& window, uint32 adapter) -> dust::context_ptr
{
	return context_ptr(new context_t(runtime, window, adapter));
}

//======================================================================
// context_t
//======================================================================
context_t::context_t(runtime_t& runtime, fooey::window_ptr const& window, uint32 adapter)
: runtime_(runtime), window_(window), current_display_mode_(), requested_display_mode_()
{
	std::tie(dxgi_adapter_, d3d_device_, d3d_immediate_context_) = platform::dxgi_and_d3d_at(runtime_, adapter);
	create_swapchain();

	// SERIOUSLY, get window client width/height instead of full width height
	setup_rendertarget(window->drawcontext_width(), window->drawcontext_height());

	bind_events(window);
}

context_t::~context_t()
{
	engine_.signal([&]{
		if (window_)
			window_->unbind(bound_events_);
	});

	engine_.signal_block();
}

auto context_t::pull_display_format(display_mode_t& mode, DXGI_SWAP_CHAIN_DESC& desc) -> void
{
	mode.width = desc.BufferDesc.Width;
	mode.height = desc.BufferDesc.Height;
	mode.fullscreen = desc.Windowed == FALSE;
	mode.format = display_format_t::r8g8b8a8_unorm;
	mode.refreshrate_frames = desc.BufferDesc.RefreshRate.Numerator;
	mode.refreshrate_period = desc.BufferDesc.RefreshRate.Denominator;
}

auto context_t::push_display_format(DXGI_MODE_DESC& dxgimode, display_mode_t const& mode) -> void
{
	dxgimode.Width = mode.width;
	dxgimode.Height = mode.height;
	dxgimode.RefreshRate = {mode.refreshrate_frames, mode.refreshrate_period};
	dxgimode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgimode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgimode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
}

auto context_t::bind_events(fooey::window_ptr const& window) -> void
{
	bound_events_ = window->on({
		{"resize-dc.dust.context", [this](fooey::events::resize_t& e) { on_resize(e); }}
	});
}

auto context_t::create_swapchain() -> void
{
	auto desc = DXGI_SWAP_CHAIN_DESC{
		// DXGI_MODE_DESC
		{0, 0, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED},
		// DXGI_SAMPLE_DESC
		{1, 0},
		DXGI_USAGE_RENDER_TARGET_OUTPUT, 1, window_->hwnd(), TRUE,
		DXGI_SWAP_EFFECT_DISCARD, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	ATMA_ENSURE_IS(S_OK, runtime_.dxgi_factory->CreateSwapChain(d3d_device_.get(), &desc, dxgi_swap_chain_.assign()));
	ATMA_ENSURE_IS(S_OK, runtime_.dxgi_factory->MakeWindowAssociation(window_->hwnd(), DXGI_MWA_NO_WINDOW_CHANGES));


	// fill our internal display-mode structures
	dxgi_swap_chain_->GetDesc(&desc); 
	pull_display_format(windowed_display_mode_, desc);
	pull_display_format(requested_windowed_display_mode_, desc);
	current_display_mode_ = &windowed_display_mode_;
}

auto context_t::setup_rendertarget(uint32 width, uint32 height) -> void
{
	// create render-target & set it
	atma::com_ptr<ID3D11Texture2D> backbuffer;
	ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.assign()));
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateRenderTargetView(backbuffer.get(), nullptr, d3d_render_target_.assign()));
	d3d_immediate_context_->OMSetRenderTargets(1, &d3d_render_target_.get_ref(), nullptr);

	D3D11_VIEWPORT vp{0, 0, (float)width, (float)height, 0, 0};
	d3d_immediate_context_->RSSetViewports(1, &vp);
}

auto context_t::recreate_backbuffer() -> void
{
	d3d_render_target_.reset();

	ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->ResizeBuffers(1, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

auto context_t::update_display_mode() -> void
{
	if (!requested_display_mode_)
		return;

	
	// transition to fullscreen
	if (current_display_mode_ == &windowed_display_mode_ && requested_display_mode_ == &requested_fullscreen_display_mode_)
	{
		DXGI_MODE_DESC mode;
		push_display_format(mode, *requested_display_mode_);
		dxgi_swap_chain_->ResizeTarget(&mode);

		recreate_backbuffer();
		setup_rendertarget(requested_display_mode_->width, requested_display_mode_->height);
		dxgi_swap_chain_->SetFullscreenState(true, dxgi_output_.get());

		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		dxgi_swap_chain_->GetDesc(&swap_chain_desc);
		pull_display_format(fullscreen_display_mode_, swap_chain_desc);

		current_display_mode_ = &fullscreen_display_mode_;
	}

	// fullscreen to fullscreen, let's not support this yet
	else if (current_display_mode_ == &fullscreen_display_mode_ && requested_display_mode_ == &requested_fullscreen_display_mode_)
	{
		ATMA_ASSERT_MSG(false, "fullscreen -> fullscreen resolution change not supported yet");
	}

	// windowed to windowed
	else if (current_display_mode_ == &windowed_display_mode_ && requested_display_mode_ == &requested_windowed_display_mode_)
	{
		recreate_backbuffer();
		setup_rendertarget(requested_windowed_display_mode_.width, requested_windowed_display_mode_.height);
	}

	// fullscreen to windowed transition
	else if (current_display_mode_ == &fullscreen_display_mode_ && requested_display_mode_ == &requested_windowed_display_mode_)
	{
		dxgi_swap_chain_->SetFullscreenState(FALSE, nullptr);

		// resize window back to what it was before fullscreening
		auto mode = DXGI_MODE_DESC{requested_windowed_display_mode_.width, requested_windowed_display_mode_.height, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED};
		dxgi_swap_chain_->ResizeTarget(&mode);

		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		dxgi_swap_chain_->GetDesc(&swap_chain_desc);
		pull_display_format(windowed_display_mode_, swap_chain_desc);

		windowed_display_mode_ = requested_windowed_display_mode_;
		current_display_mode_ = &windowed_display_mode_;
	}

	requested_display_mode_ = nullptr;
}

auto context_t::signal_block() -> void
{
	engine_.signal_block();
}

auto context_t::signal_fullscreen_toggle(uint32 output_index) -> void
{
	engine_.signal([&, output_index]
	{
		ATMA_ASSERT(dxgi_adapter_);

		// going to fullscreen
		if (current_display_mode_ == &windowed_display_mode_)
		{
			// get output of our adapter
			dxgi_output_ = platform::output_at(runtime_, dxgi_adapter_, output_index);
			ATMA_ASSERT(dxgi_output_);

			// find best-fitting fullscreen resolution
			DXGI_OUTPUT_DESC output_desc;
			dxgi_output_->GetDesc(&output_desc);

			auto candidate = DXGI_MODE_DESC{800, 600, {0, 0}, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED};
			auto mode = DXGI_MODE_DESC{};
			dxgi_output_->FindClosestMatchingMode(&candidate, &mode, d3d_device_.get());

			DXGI_SWAP_CHAIN_DESC swap_chain_desc;
			dxgi_swap_chain_->GetDesc(&swap_chain_desc);
			swap_chain_desc.BufferDesc = mode;

			pull_display_format(requested_fullscreen_display_mode_, swap_chain_desc);

			requested_display_mode_ = &requested_fullscreen_display_mode_;
		}
		else
		{
			requested_display_mode_ = &requested_windowed_display_mode_;
		}
	});
}

auto context_t::on_resize(fooey::events::resize_t& e) -> void
{
	requested_windowed_display_mode_.width = e.width();
	requested_windowed_display_mode_.height = e.height();

	if (requested_display_mode_ == &requested_fullscreen_display_mode_)
		return;

	requested_display_mode_ = &requested_windowed_display_mode_;

#if 0
	engine_.signal([&, e] {
		recreate_backbuffer();
		setup_rendertarget(e.width(), e.height());
	});
#endif
}

auto context_t::create_d3d_buffer(platform::d3d_buffer_ptr& buffer, gpu_access_t gpu_access, cpu_access_t cpu_access, size_t data_size, void* data) -> void
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

	D3D11_BUFFER_DESC buffer_desc{(UINT)data_size, buffer_usage, D3D11_BIND_VERTEX_BUFFER, cpua, 0, 0};


	if (data) {
		// for vertex buffers, the pitch and slice-pitch mean nothing, so we'll just
		// pass along stats to help with debugging (1xdata_size buffer created)
		D3D11_SUBRESOURCE_DATA d3d_data{data, 1, (UINT)data_size};
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateBuffer(&buffer_desc, &d3d_data, buffer.assign()));
	}
	else {
		ATMA_ENSURE_IS(S_OK, d3d_device_->CreateBuffer(&buffer_desc, NULL, buffer.assign()));
	}
}

auto context_t::signal_d3d_map(platform::d3d_buffer_ptr& buffer, D3D11_MAPPED_SUBRESOURCE* mapped_resource, D3D11_MAP map_type, uint32 subresource, std::function<void(D3D11_MAPPED_SUBRESOURCE*)> const& fn) -> void
{
	engine_.signal([&, mapped_resource, map_type, subresource] {
		d3d_immediate_context_->Map(buffer.get(), subresource, map_type, 0, mapped_resource);
		if (fn)
			fn(mapped_resource);
	});
}

auto context_t::signal_d3d_unmap(platform::d3d_buffer_ptr& buffer, uint32 subresource) -> void
{
	engine_.signal([&, buffer, subresource] {
		d3d_immediate_context_->Unmap(buffer.get(), subresource);
	});
}

auto context_t::signal_d3d_buffer_upload(platform::d3d_buffer_ptr& buffer, void const* data, uint32 row_pitch, uint32 depth_pitch) -> void
{
	engine_.signal([&, buffer, data, row_pitch, depth_pitch] {
		d3d_immediate_context_->UpdateSubresource(buffer.get(), 0, nullptr, data, row_pitch, depth_pitch);
	});
}

auto context_t::signal_draw(vertex_declaration_t const& vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, pixel_shader_ptr const& ps) -> void
{
	engine_.signal([&, vd, vb, vs, ps]{
		UINT stride = sizeof(float) * 4;
		UINT offset = 0;

		auto vbs = vb->d3d_buffer().get();

		d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
		d3d_immediate_context_->PSSetShader(ps->d3d_ps().get(), nullptr, 0);
		d3d_immediate_context_->IASetInputLayout(vd.d3d_input_layout().get());
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vbs, &stride, &offset);
		d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3d_immediate_context_->Draw(3, 0);
	});
}

auto context_t::signal_present() -> void
{
	engine_.signal([&] {
		ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->Present(DXGI_SWAP_EFFECT_DISCARD, 0));

		update_display_mode();
	});
}

auto context_t::signal_clear() -> void
{
	engine_.signal([&] {
		float g[4] ={.2f, .2f, .2f, 1.f};
		d3d_immediate_context_->ClearRenderTargetView(d3d_render_target_.get(), g);
	});
}
