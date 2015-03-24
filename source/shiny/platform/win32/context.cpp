#include <shiny/platform/win32/context.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>
#include <shiny/platform/win32/dxgi_fwd.hpp>

#include <shiny/data_declaration.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>
#include <shiny/runtime.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/scene.hpp>
#include <shiny/compute_shader.hpp>
#include <shiny/texture2d.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/shader_resource2d.hpp>
#include <shiny/generic_buffer.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>

#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>

#include <vector>
#include <atomic>
#include <map>


using namespace shiny;
using shiny::context_t;


//======================================================================
// context_t
//======================================================================
auto shiny::create_context(runtime_t& runtime, fooey::window_ptr const& window, uint adapter) -> shiny::context_ptr {
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
		{"resize-dc.shiny.context", [this](fooey::events::resize_t& e) { on_resize(e); }}
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
	// remove reference to old render-target
	d3d_immediate_context_->OMSetRenderTargets(0, nullptr, nullptr);

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
		// 1. call ResizeTarget, which resizes our win32 target, generating a 'resize' event.
		//    NOTE: not a 'resize-dc' event.
		DXGI_MODE_DESC mode;
		push_display_format(mode, *requested_display_mode_);
		dxgi_swap_chain_->ResizeTarget(&mode);
		
		// 2. recreates the back-buffers, since ResizeTarget has dealt with the front-buffers.
		// 3. our dxgi render-target-view is now invalid, so retrieve the new one and give it to d3d
		recreate_backbuffer();
		setup_rendertarget(requested_display_mode_->width, requested_display_mode_->height);

		// 4. *now* switch to fullscreen - this doesn't change any of the buffers, see. it simply
		//    changes to mode as far as I know.
		dxgi_swap_chain_->SetFullscreenState(true, dxgi_output_.get());
		
		// 5. finally, simply get the display-mode information
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

auto context_t::signal_draw(data_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, fragment_shader_ptr const& ps) -> void
{
	engine_.signal([&, vd, vb, vs, ps]
	{
#if 0
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ID3D11DepthStencilState *m_DepthStencilState;
		d3d_device_->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
		d3d_immediate_context_->OMSetDepthStencilState(m_DepthStencilState, 0);
#endif

		UINT stride = vd->stride();
		UINT offset = 0;

		// input-layout
		auto ILkey = std::make_tuple(vs, vd);
		auto IL = cached_input_layouts_.find(ILkey);
		if (IL == cached_input_layouts_.end()) {
			IL = cached_input_layouts_.insert(std::make_pair(ILkey, create_d3d_input_layout(vs, vd))).first;
		}

		auto vbs = vb->d3d_buffer().get();

		d3d_immediate_context_->IASetInputLayout(IL->second.get());
		d3d_immediate_context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vbs, &stride, &offset);
		//d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
		d3d_immediate_context_->PSSetShader(ps->d3d_ps().get(), nullptr, 0);

		d3d_immediate_context_->Draw(vb->vertex_count(), 0);
	});
}

auto context_t::signal_draw(index_buffer_ptr const& ib, data_declaration_t const* vd, vertex_buffer_ptr const& vb, vertex_shader_ptr const& vs, fragment_shader_ptr const& ps) -> void
{
	engine_.signal([&, ib, vd, vb, vs, ps]{
		UINT stride = vd->stride();
		UINT offset = 0;

		auto vbs = vb->d3d_buffer().get();

		D3D11_RASTERIZER_DESC wfdesc;
		ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
		wfdesc.FillMode = D3D11_FILL_SOLID;
		wfdesc.CullMode = D3D11_CULL_NONE;
		atma::com_ptr<ID3D11RasterizerState> WireFrame;
		d3d_device_->CreateRasterizerState(&wfdesc, WireFrame.assign());
		d3d_immediate_context_->RSSetState(WireFrame.get());

#if 0
		D3D11_BLEND_DESC blendStateDesc;
		ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
		blendStateDesc.AlphaToCoverageEnable = FALSE;
		blendStateDesc.IndependentBlendEnable = FALSE;
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		auto blendState = atma::com_ptr<ID3D11BlendState>{};
		if (FAILED(d3d_device_->CreateBlendState(&blendStateDesc, blendState.assign()))) {
			printf("Failed To Create Blend State\n");
		}
		d3d_immediate_context_->OMSetBlendState(blendState.get(), NULL, 0xFFFFFF);
#endif


		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		ID3D11DepthStencilState *m_DepthStencilState;
		d3d_device_->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
		d3d_immediate_context_->OMSetDepthStencilState(m_DepthStencilState, 0);



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

auto context_t::signal_clear(atma::math::vector4f const& color) -> void
{
	engine_.signal([&] {
		float f4c[4] = {color.x, color.y, color.z, color.w};
		d3d_immediate_context_->ClearRenderTargetView(d3d_render_target_.get(), f4c);
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

auto context_t::create_d3d_input_layout(vertex_shader_ptr const& vs, data_declaration_t const* vd) -> platform::d3d_input_layout_ptr
{
	uint offset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	for (auto const& x : vd->streams())
	{
		d3d_elements.push_back({
			x.semantic().c_str(), 0,
			platform::dxgi_format_of(x.element_format()),
			0, offset,
			platform::d3d_input_class_of(x.stage()), 0
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

auto context_t::signal_draw(shared_state_t const& ss, vertex_stage_state_t const& vss, fragment_stage_state_t const& fss) -> void
{
	engine_.signal([&, ss, vss, fss]
	{
		auto const& vs = vss.vertex_shader;
		auto const& vb = vss.vertex_buffer;
		auto const* vd = vb->data_declaration();

		auto const& fs = fss.fragment_shader;

		// input assembler
		ATMA_ASSERT(vs->data_declaration() == vss.vertex_buffer->data_declaration());

		UINT offset = 0, stride = vd->stride();
		d3d_immediate_context_->IASetInputLayout(vs->d3d_input_layout().get());
		d3d_immediate_context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		d3d_immediate_context_->IASetVertexBuffers(0, 1, &vb->d3d_buffer().get(), &stride, &offset);
		d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// vertex-shader
		d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
		for (auto const& x : ss.shader_resources) d3d_immediate_context_->VSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());
		//for (auto const& x : vss.shader_resources) d3d_immediate_context_->VSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());

		// fragment-shader
		d3d_immediate_context_->PSSetShader(fs->d3d_ps().get(), nullptr, 0);
		for (auto const& x : ss.shader_resources) d3d_immediate_context_->PSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());
		for (auto const& x : fss.shader_resources) d3d_immediate_context_->PSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());

		auto vertex_count = (vss.count == 0) ? vb->vertex_count() : vss.count;
		d3d_immediate_context_->Draw(vertex_count, vss.offset);
	});
}


auto context_t::signal_gs_set(geometry_shader_ptr const& gs) -> void
{
	engine_.signal([&, gs]{
		d3d_immediate_context_->GSSetShader(gs->d3d_gs().get(), nullptr, 0);
	});
}



auto context_t::make_blender(blend_state_t const& bs) -> blender_ptr
{
	// cached?
	auto candidate = cached_blenders_.find(bs);
	if (candidate != cached_blenders_.end())
		return candidate->second;

	// not cached; construct
	auto d3d_blend_desc = D3D11_BLEND_DESC{};
	d3d_blend_desc.AlphaToCoverageEnable = false;
	d3d_blend_desc.IndependentBlendEnable = !bs.multitarget_collapse;
	for (int i = 0; i != 8; ++i)
	{
		auto const& rt = bs.rendertarget[i];
		auto& d3drt = d3d_blend_desc.RenderTarget[i];

		d3drt.BlendEnable = rt.blending_enabled;
		d3drt.DestBlend = platform::d3dblend_of(rt.dest_blend);
		d3drt.DestBlendAlpha = platform::d3dblend_of(rt.dest_blend_alpha);
		d3drt.SrcBlend = platform::d3dblend_of(rt.src_blend);
		d3drt.SrcBlendAlpha = platform::d3dblend_of(rt.src_blend_alpha);
		d3drt.BlendOp = D3D11_BLEND_OP_ADD;
		d3drt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		d3drt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	platform::d3d_blend_state_ptr d3d_bs;

	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateBlendState(&d3d_blend_desc, d3d_bs.assign()));

	auto thing = blender_ptr(new blender_t(shared_from_this<context_t>(), d3d_bs));

	cached_blenders_[bs] = thing;
	return thing;
}

auto context_t::signal_om_blending(blender_cptr const& b) -> void
{
	engine_.signal([&, b]{
		d3d_immediate_context_->OMSetBlendState(b->d3d_blend_state().get(), nullptr, 0xffffffff);
	});
}

auto context_t::signal_ia_topology(topology_t t) -> void
{
	engine_.signal([&, t]{
		auto d3dt =
			(t == topology_t::point) ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST :
			(t == topology_t::line) ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST :
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		d3d_immediate_context_->IASetPrimitiveTopology(d3dt);
	});
}

auto context_t::setup_debug_geometry() -> void
{
}


