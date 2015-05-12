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
#include <shiny/generic_buffer.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>

#include <fooey/events/resize.hpp>
#include <fooey/keys.hpp>
#include <fooey/widgets/window.hpp>

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

auto context_t::signal_d3d_buffer_upload(platform::d3d_buffer_ptr const& buffer, void const* data, uint row_pitch, uint depth_pitch) -> void
{
	engine_.signal([&, buffer, data, row_pitch, depth_pitch] {
		d3d_immediate_context_->UpdateSubresource(buffer.get(), 0, nullptr, data, row_pitch, depth_pitch);
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

auto context_t::signal_draw_scene(scene_t& scene) -> void
{
	engine_.signal_batch(scene.batch_);
}

auto context_t::signal(atma::thread::engine_t::queue_t::batch_t& batch) -> void
{
	engine_.signal_batch(batch);
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

auto context_t::immediate_clear(rendertarget_clear_t const& rtc) -> void
{
	float color[4] = {rtc.color().x, rtc.color().y, rtc.color().z, rtc.color().w};
	d3d_immediate_context_->ClearRenderTargetView(d3d_render_target_.get(), color);
	d3d_immediate_context_->ClearDepthStencilView(d3d_depth_stencil_.get(), D3D11_CLEAR_DEPTH, rtc.depth(), rtc.stencil());
}

auto context_t::immediate_draw_pipeline_reset() -> void
{
	draw_range_ = draw_range_t{};
}

auto context_t::immediate_ia_set_data_declaration(data_declaration_t const* dd) -> void
{
	data_declaration_ = dd;
}

auto context_t::immediate_ia_set_vertex_buffer(vertex_buffer_cptr const& vb) -> void
{
	ATMA_ASSERT(vb->data_declaration() == data_declaration_);

	UINT stride = (UINT)vb->data_declaration()->stride(), offset = 0;
	d3d_immediate_context_->IASetVertexBuffers(0, 1, &vb->d3d_buffer().get(), &stride, &offset);
}

auto context_t::immediate_ia_set_index_buffer(index_buffer_cptr const& ib) -> void
{
	index_buffer_ = ib;
	auto fmt = platform::dxgi_format_of(ib->index_format());
	d3d_immediate_context_->IASetIndexBuffer(ib->d3d_buffer().get(), fmt, 0);
}

auto context_t::immediate_ia_set_topology(topology_t t) -> void
{
	auto d3dt =
		(t == topology_t::point) ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST :
		(t == topology_t::line) ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST :
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	d3d_immediate_context_->IASetPrimitiveTopology(d3dt);
}

auto context_t::immediate_gs_set_geometry_shader(geometry_shader_cptr const& gs) -> void
{
	d3d_immediate_context_->GSSetShader(gs->d3d_gs().get(), nullptr, 0);
}

auto context_t::immediate_vs_set_vertex_shader(vertex_shader_cptr const& vs) -> void
{
	vertex_shader_ = vs;
	d3d_immediate_context_->VSSetShader(vs->d3d_vs().get(), nullptr, 0);
}

auto context_t::immediate_vs_set_constant_buffers(bound_constant_buffers_t const& cbs) -> void
{
	for (auto const& x : cbs) {
		d3d_immediate_context_->VSSetConstantBuffers(x.first, 1, &x.second->d3d_buffer().get());
	}
}

auto context_t::immediate_vs_set_resources(bound_resources_t const& rs) -> void
{
	for (auto const& x : rs) {
		//d3d_immediate_context_->VSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());
	}
}

auto context_t::immediate_fs_set_fragment_shader(fragment_shader_cptr const& fs) -> void
{
	d3d_immediate_context_->PSSetShader(fs->d3d_fs().get(), nullptr, 0);
}

auto context_t::immediate_fs_set_constant_buffers(bound_constant_buffers_t const& cbs) -> void
{
	for (auto const& x : cbs) {
		d3d_immediate_context_->PSSetConstantBuffers(x.first, 1, &x.second->d3d_buffer().get());
	}
}

auto context_t::immediate_fs_set_resources(bound_resources_t const& rs) -> void
{
	for (auto const& x : rs) {
		//d3d_immediate_context_->PSSetShaderResources(x.first, 1, &x.second->d3d_srv().get());
	}
}

auto context_t::immediate_draw_set_range(draw_range_t const& dr) -> void
{
	draw_range_ = dr;
}

auto context_t::immediate_om_set_blending(blender_cptr const& b) -> void
{
	d3d_immediate_context_->OMSetBlendState(b->d3d_blend_state().get(), nullptr, 0xffffffff);
}

auto context_t::immediate_draw() -> void
{
	auto ILkey = std::make_tuple(vertex_shader_, data_declaration_);
	auto IL = cached_input_layouts_.find(ILkey);
	if (IL == cached_input_layouts_.end()) {
		IL = cached_input_layouts_.insert(std::make_pair(ILkey, create_d3d_input_layout(vertex_shader_, data_declaration_))).first;
	}

	d3d_immediate_context_->IASetInputLayout(IL->second.get());

	if (index_buffer_)
	{
		if (draw_range_.index_count == 0)
			draw_range_.index_count = index_buffer_->index_count();

		d3d_immediate_context_->DrawIndexed(draw_range_.index_count, draw_range_.index_offset, draw_range_.vertex_offset);
	}
	else
	{
		d3d_immediate_context_->Draw(draw_range_.vertex_count, draw_range_.vertex_offset);
	}
}

auto context_t::immediate_compute_pipeline_reset() -> void
{
	engine_.signal([&]{
		auto const cb_count = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		ID3D11Buffer* bufs[cb_count] ={};

		d3d_immediate_context_->CSSetConstantBuffers(0, cb_count, bufs);
	});
}

auto context_t::immediate_cs_set_constant_buffers(bound_constant_buffers_t const& bufs) -> void
{
	engine_.signal([&, bufs]
	{
		auto const count = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
		ID3D11Buffer* dbufs[count] ={};
		
		for (auto const& x : bufs)
			dbufs[x.first] = x.second->d3d_buffer().get();

		d3d_immediate_context_->CSSetConstantBuffers(0, count, dbufs);
	});
}

auto context_t::immediate_cs_set_input_views(bound_resource_views_t const& views) -> void
{
	engine_.signal([&, views]
	{
		auto const count = 16;
		ID3D11ShaderResourceView* srvs[16] = {};
		
		for (auto const& x : views)
			srvs[x.first] = (ID3D11ShaderResourceView*)x.second->d3d_view().get();

		d3d_immediate_context_->CSSetShaderResources(0, count, srvs);
	});
}

auto context_t::immediate_cs_set_compute_views(bound_resource_views_t const& views) -> void
{
	engine_.signal([&, views]
	{
		auto const count = D3D11_PS_CS_UAV_REGISTER_COUNT;
		ID3D11UnorderedAccessView* uavs[count] ={};

		for (auto const& x : views)
			uavs[x.first] = (ID3D11UnorderedAccessView*)x.second->d3d_view().get();

		d3d_immediate_context_->CSSetUnorderedAccessViews(0, count, uavs, nullptr);
	});
}

auto context_t::immediate_cs_set_compute_shader(compute_shader_cptr const& cs) -> void
{
	engine_.signal([&, cs]{
		d3d_immediate_context_->CSSetShader(cs->d3d_cs().get(), nullptr, 0);
	});
}

auto context_t::immediate_compute(uint x, uint y, uint z) -> void
{
	engine_.signal([&, x, y, z]{
#if 0
		atma::com_ptr<ID3D11Query> query;
		auto desc = D3D11_QUERY_DESC{D3D11_QUERY_EVENT, 0};
		d3d_device_->CreateQuery(&desc, query.assign());
		//d3d_immediate_context_->Begin(query.get());
		//d3d_immediate_context_->End(query.get());
		BOOL result;
		while (S_OK != d3d_immediate_context_->GetData(query.get(), &result, sizeof(BOOL), 0))
		{
			std::cout << "not finished" << std::endl;
		}
#endif
		d3d_immediate_context_->Dispatch(x, y, z);
#if 0
		auto const count = D3D11_PS_CS_UAV_REGISTER_COUNT;
		ID3D11UnorderedAccessView* uavs[count] ={};
		d3d_immediate_context_->CSSetUnorderedAccessViews(0, count, uavs, nullptr);
#endif // 0

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
		auto sr = D3D11_MAPPED_SUBRESOURCE{};
		ATMA_ENSURE_IS(S_OK, d3d_immediate_context_->Map(rs->d3d_resource().get(), subresource, d3dmap, 0, &sr));
		mapped_subresource_t msr{sr.pData, sr.RowPitch, sr.DepthPitch};
		fn(msr);
		d3d_immediate_context_->Unmap(rs->d3d_resource().get(), subresource);
	});
}

auto context_t::create_d3d_input_layout(vertex_shader_cptr const& vs, data_declaration_t const* vd) -> platform::d3d_input_layout_ptr
{
	size_t offset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	for (auto const& x : vd->streams())
	{
		d3d_elements.push_back({
			x.semantic().c_str(), 0,
			platform::dxgi_format_of(x.element_format()),
			0, (UINT)offset,
			platform::d3d_input_class_of(x.stage()), 0
		});

		offset += x.size();
	}

	platform::d3d_input_layout_ptr result;
	ATMA_ENSURE_IS(S_OK, d3d_device_->CreateInputLayout(&d3d_elements[0], (uint)d3d_elements.size(),
		vs->d3d_blob()->GetBufferPointer(), vs->d3d_blob()->GetBufferSize(), result.assign()));

	return result;
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

auto context_t::make_generic_buffer(resource_usage_mask_t const& rs, resource_storage_t bu, size_t stride, uint elements, void const* data, uint data_elemcount) -> generic_buffer_ptr
{
	return generic_buffer_ptr(new generic_buffer_t(shared_from_this<context_t>(), rs, bu, stride, elements, data, data_elemcount));
}

auto context_t::on_resize(fooey::events::resize_t& e) -> void
{
	if (requested_display_mode_ == &requested_fullscreen_display_mode_)
		return;

	requested_windowed_display_mode_.width = e.width();
	requested_windowed_display_mode_.height = e.height();

	requested_display_mode_ = &requested_windowed_display_mode_;
}

auto context_t::signal_copy_buffer(resource_ptr const& dest, resource_cptr const& src) -> void
{
	engine_.signal([&, dest, src]{
		d3d_immediate_context_->CopyResource(dest->d3d_resource().get(), src->d3d_resource().get());
		//d3d_immediate_context_->CopySubresourceRegion(dest->d3d_resource().get(), 0, 0, 0, 0, src->d3d_resource().get(), 0, nullptr);
		//d3d_immediate_context_->Flush();
	});
}
