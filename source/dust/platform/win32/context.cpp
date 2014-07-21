#include <dust/context.hpp>

#include <dust/platform/win32/dxgid3d_convert.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/platform/win32/dxgi_fwd.hpp>

#include <dust/vertex_declaration.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/runtime.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/index_buffer.hpp>
#include <dust/scene.hpp>
#include <dust/compute_shader.hpp>
#include <dust/texture2d.hpp>
#include <dust/texture3d.hpp>
#include <dust/shader_resource2d.hpp>
#include <dust/generic_buffer.hpp>
#include <dust/vertex_buffer.hpp>

#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>

#include <vector>
#include <atomic>
#include <map>


using namespace dust;
using dust::context_t;


//======================================================================
// context_t
//======================================================================
auto dust::create_context(runtime_t& runtime, fooey::window_ptr const& window, uint adapter) -> dust::context_ptr {
	return context_ptr(new context_t(runtime, window, adapter));
}

context_t::context_t(runtime_t& runtime, fooey::window_ptr const& window, uint adapter)
: runtime_(runtime), window_(window), current_display_mode_(), requested_display_mode_()
{
	std::tie(dxgi_adapter_, d3d_device_, d3d_immediate_context_) = runtime_.dxgid3d_for_adapter(adapter);
	create_swapchain();

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
	mode.format = element_format_t::un8x4;
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
	/*bound_events_ = */window->on({
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

	ATMA_ENSURE_IS(S_OK, runtime_.dxgi_factory()->CreateSwapChain(d3d_device_.get(), &desc, dxgi_swap_chain_.assign()));
	ATMA_ENSURE_IS(S_OK, runtime_.dxgi_factory()->MakeWindowAssociation(window_->hwnd(), DXGI_MWA_NO_WINDOW_CHANGES));


	// fill our internal display-mode structures
	dxgi_swap_chain_->GetDesc(&desc); 
	pull_display_format(windowed_display_mode_, desc);
	pull_display_format(requested_windowed_display_mode_, desc);
	current_display_mode_ = &windowed_display_mode_;
}

auto context_t::setup_rendertarget(uint width, uint height) -> void
{
	// create render-target
	atma::com_ptr<ID3D11Texture2D> backbuffer;
	ATMA_ENSURE_IS(S_OK, dxgi_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.assign()));
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateRenderTargetView(backbuffer.get(), nullptr, d3d_render_target_.assign()));
	
	// create depth-stencil buffer & depth-stencil
	D3D11_TEXTURE2D_DESC texdesc{width, height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, 0};
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateTexture2D(&texdesc, nullptr, d3d_depth_stencil_buffer_.assign()));
	D3D11_DEPTH_STENCIL_VIEW_DESC depthdesc{texdesc.Format, D3D11_DSV_DIMENSION_TEXTURE2D, {0}};
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateDepthStencilView(d3d_depth_stencil_buffer_.get(), &depthdesc, d3d_depth_stencil_.assign()));

	// set render targets
	d3d_immediate_context_->OMSetRenderTargets(1, &d3d_render_target_.get(), d3d_depth_stencil_.get());

	// create viewport
	D3D11_VIEWPORT vp{0, 0, (float)width, (float)height, 0, 1.f};
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

auto context_t::signal_fullscreen_toggle(uint output_index) -> void
{
	engine_.signal([&, output_index]
	{
		ATMA_ASSERT(dxgi_adapter_);

		// going to fullscreen
		if (current_display_mode_ == &windowed_display_mode_)
		{
			// get output of our adapter
			dxgi_output_ = runtime_.dxgi_output_of(dxgi_adapter_, output_index);
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
	if (requested_display_mode_ == &requested_fullscreen_display_mode_)
		return;

	requested_windowed_display_mode_.width = e.width();
	requested_windowed_display_mode_.height = e.height();

	requested_display_mode_ = &requested_windowed_display_mode_;
}

auto context_t::signal_d3d_buffer_upload(platform::d3d_buffer_ptr const& buffer, void const* data, uint row_pitch, uint depth_pitch) -> void
{
	engine_.signal([&, buffer, data, row_pitch, depth_pitch] {
		d3d_immediate_context_->UpdateSubresource(buffer.get(), 0, nullptr, data, row_pitch, depth_pitch);
	});
}

auto context_t::signal_draw(vertex_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, pixel_shader_ptr const& ps) -> void
{
	engine_.signal([&, vd, vb, vs, ps]{
		UINT stride = vd->stride();
		UINT offset = 0;

		auto vbs = vb->d3d_buffer().get();

		// input-layout
		auto ILkey = std::make_tuple(vs, vd);
		auto IL = cached_input_layouts_.find(ILkey);
		if (IL == cached_input_layouts_.end()) {
			IL = cached_input_layouts_.insert(std::make_pair(ILkey, create_d3d_input_layout(vs, vd))).first;
		}

		d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
		d3d_immediate_context_->PSSetShader(ps->d3d_ps().get(), nullptr, 0);
		d3d_immediate_context_->IASetInputLayout(IL->second.get());
		d3d_immediate_context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vbs, &stride, &offset);
		d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3d_immediate_context_->Draw(vb->vertex_count(), 0);
	});
}

auto context_t::signal_draw(index_buffer_ptr const& ib, vertex_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, pixel_shader_ptr const& ps) -> void
{
	engine_.signal([&, ib, vd, vb, vs, ps]{
		UINT stride = vd->stride();
		UINT offset = 0;

		auto vbs = vb->d3d_buffer().get();

		// input-layout
		auto ILkey = std::make_tuple(vs, vd);
		auto IL = cached_input_layouts_.find(ILkey);
		if (IL == cached_input_layouts_.end()) {
			IL = cached_input_layouts_.insert(std::make_pair(ILkey, create_d3d_input_layout(vs, vd))).first;
		}

		d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
		d3d_immediate_context_->PSSetShader(ps->d3d_ps().get(), nullptr, 0);
		d3d_immediate_context_->IASetInputLayout(IL->second.get());
		d3d_immediate_context_->IASetIndexBuffer(ib->d3d_buffer().get(), DXGI_FORMAT_R16_UINT, 0);
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vbs, &stride, &offset);
		d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3d_immediate_context_->DrawIndexed(ib->index_count(), 0, 0);
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
		d3d_immediate_context_->ClearDepthStencilView(d3d_depth_stencil_.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	});
}

auto context_t::signal_cs_upload_constant_buffer(uint index, constant_buffer_cptr const& buf) -> void
{
	engine_.signal([&, index, buf] {
		d3d_immediate_context_->VSSetConstantBuffers(index, 1, &buf->d3d_buffer().get());
		d3d_immediate_context_->PSSetConstantBuffers(index, 1, &buf->d3d_buffer().get());
	});
}

auto context_t::signal_draw_scene(scene_t& scene) -> void
{
	scene.execute();
}

auto context_t::signal_res_update(constant_buffer_ptr const& cb, uint data_size, void* data) -> void
{
	signal_res_update(cb, atma::shared_memory_t(data_size, data));
}

auto context_t::signal_res_update(constant_buffer_ptr const& cb, atma::shared_memory_t const& sm) -> void
{
	engine_.signal([&, cb, sm] {
		D3D11_MAPPED_SUBRESOURCE sr;
		d3d_immediate_context_->Map(cb->d3d_resource().get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sr);
		memcpy(sr.pData, sm.begin(), sm.size());
		d3d_immediate_context_->Unmap(cb->d3d_resource().get(), 0);
	});
}

auto context_t::signal_cs_set(compute_shader_ptr const& cs) -> void
{
	engine_.signal([&, cs] {
		d3d_immediate_context_->CSSetShader(cs->d3d_cs().get(), nullptr, 0);
	});
}


auto context_t::signal_cs_upload_shader_resource(view_type_t view_type, shader_resource2d_ptr const& rs) -> void
{
	if (view_type == view_type_t::read_only) {
		engine_.signal([&, rs] {
			d3d_immediate_context_->CSSetShaderResources(0, 1, &(ID3D11ShaderResourceView*&)rs->d3d_view().get());
		});
	}
	else {
		engine_.signal([&, rs] {
			d3d_immediate_context_->CSSetUnorderedAccessViews(0, 1, &(ID3D11UnorderedAccessView*&)rs->d3d_view().get(), nullptr);
		});
	}
}

auto context_t::signal_cs_dispatch(uint x, uint y, uint z) -> void
{
	engine_.signal([&, x, y, z]{
		d3d_immediate_context_->Dispatch(x, y, z);
	});
}

auto context_t::signal_res_map(resource_ptr const& rs, uint subresource, map_type_t maptype, map_callback_t const& fn) -> void
{
	auto d3dmap = 
		maptype == map_type_t::read ? D3D11_MAP_READ :
		maptype == map_type_t::write ? D3D11_MAP_WRITE :
		maptype == map_type_t::write_discard ? D3D11_MAP_WRITE_DISCARD : 
		D3D11_MAP_READ_WRITE
		;

	engine_.signal([&, rs, subresource, d3dmap, fn] {
		D3D11_MAPPED_SUBRESOURCE sr;
		ATMA_ENSURE_IS(S_OK, d3d_immediate_context_->Map(rs->d3d_resource().get(), subresource, d3dmap, 0, &sr));
		mapped_subresource_t msr{sr.pData, sr.RowPitch, sr.DepthPitch};
		fn(msr);
		d3d_immediate_context_->Unmap(rs->d3d_resource().get(), subresource);
	});
}

#if 0
auto context_t::signal_cs_upload_generic_buffer(uint index, generic_buffer_ptr const& buf) -> void
{
	engine_.signal([&, index, buf] {
		d3d_immediate_context_->CSSetShaderResources(index, 1, &buf->d3d_shader_resource_view().get());
	});
}
#endif

auto context_t::signal_fs_upload_shader_resource(uint index, resource_ptr const& resource) -> void
{
	ATMA_ASSERT(resource);
	engine_.signal([&, index, resource] {
		d3d_immediate_context_->PSSetShaderResources(index, 1, &resource->d3d_srv().get());
	});
}

auto context_t::create_d3d_input_layout(vertex_shader_ptr const& vs, vertex_declaration_t const* vd) -> platform::d3d_input_layout_ptr
{
	uint offset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	for (auto const& x : vd->streams())
	{
		d3d_elements.push_back({
			x.semantic() == vertex_stream_semantic_t::position ? "Position" : "Color",
			0, platform::dxgi_format_of(x.element_format()), 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0
		});

		offset += x.size();
	}

	platform::d3d_input_layout_ptr result;
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateInputLayout(&d3d_elements[0], (uint)d3d_elements.size(),
		vs->d3d_blob()->GetBufferPointer(), vs->d3d_blob()->GetBufferSize(), result.assign()));

	return result;
}

auto context_t::signal_vs_upload_constant_buffer(uint index, constant_buffer_cptr const& cb) -> void
{
	engine_.signal([&, index, cb] {
		d3d_immediate_context_->VSSetConstantBuffers(index, 1, &cb->d3d_buffer().get());
	});
}

auto context_t::signal_fs_upload_constant_buffer(uint index, constant_buffer_cptr const& cb) -> void
{
	engine_.signal([&, index, cb] {
		d3d_immediate_context_->PSSetConstantBuffers(index, 1, &cb->d3d_buffer().get());
	});
}

auto context_t::signal_draw(shared_state_t const& ss, vertex_stage_state_t const& vs, fragment_stage_state_t const& fs) -> void
{
	engine_.signal([&, ss, vs, fs] {
		//for (auto const& x : ss.shader_resources())
		UINT stride = vs.vertex_declaration->stride();
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vs.vertex_buffer->d3d_buffer().get(), &stride, 0);
		d3d_immediate_context_->VSSetShader(vs.vertex_shader->d3d_vs().get(), nullptr, 0);

		d3d_immediate_context_->PSSetShader(fs.fragment_shader->d3d_ps().get(), nullptr, 0);
	});
}
