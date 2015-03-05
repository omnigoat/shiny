#pragma once
//======================================================================
#include <shiny/dust_fwd.hpp>
#include <shiny/element_format.hpp>
#include <shiny/adapter.hpp>
#include <shiny/output.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/geometry_shader.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>

#include <shiny/platform/win32/d3d_fwd.hpp>

#include <fooey/fooey_fwd.hpp>
#include <fooey/event_handler.hpp>

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/thread/engine.hpp>
#include <atma/shared_memory.hpp>
#include <atma/math/vector4f.hpp>

#include <thread>
#include <mutex>
//======================================================================
namespace shiny {
//======================================================================

	namespace constant_buffer_index
	{
		uint const user = 3;
	}

	struct shared_state_t
	{
		typedef std::vector<std::pair<uint, resource_ptr>> shader_resources_t;

		bound_resources_t shader_resources;
	};

	struct vertex_stage_state_t
	{
		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib, uint offset, uint count, bound_resources_t const& sr)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(offset), count(count), shader_resources(sr)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib, uint offset, uint count)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(offset), count(count)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, index_buffer_cptr const& ib)
			: vertex_shader(vs), vertex_buffer(vb), index_buffer(ib), offset(), count()
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb, uint offset, uint count)
			: vertex_shader(vs), vertex_buffer(vb), offset(offset), count(count)
		{}

		vertex_stage_state_t(vertex_shader_cptr const& vs, vertex_buffer_cptr const& vb)
			: vertex_shader(vs), vertex_buffer(vb), offset(), count()
		{}

		vertex_shader_cptr vertex_shader;
		vertex_buffer_cptr vertex_buffer;
		index_buffer_cptr index_buffer;
		bound_resources_t shader_resources;

		uint offset, count;
	};

	struct fragment_stage_state_t
	{
		fragment_stage_state_t(fragment_shader_cptr const& fs, bound_constant_buffers_t const& bcb, bound_resources_t const& bs)
			: fragment_shader(fs), constant_buffers(bcb), shader_resources(bs)
		{}

		fragment_stage_state_t(fragment_shader_cptr const& fs, bound_constant_buffers_t const& bcb)
			: fragment_shader(fs), constant_buffers(bcb)
		{}

		fragment_stage_state_t(fragment_shader_cptr const& fs)
			: fragment_shader(fs)
		{}

		fragment_shader_cptr fragment_shader;
		bound_constant_buffers_t constant_buffers;
		bound_resources_t shader_resources;
	};




	struct context_t : atma::ref_counted
	{
		using map_callback_t = std::function<void(mapped_subresource_t&)>;

		~context_t();

		auto runtime() -> runtime_t& { return runtime_; }
		auto runtime() const -> runtime_t const& { return runtime_; }

		auto signal_block() -> void;
		auto signal_fullscreen_toggle(uint output_index = primary_output) -> void;
		auto signal_present() -> void;
		auto signal_clear(atma::math::vector4f const&) -> void;
		auto signal_draw(vertex_declaration_t const*, vertex_buffer_ptr const&, vertex_shader_ptr const&, fragment_shader_ptr const&) -> void;
		auto signal_draw(index_buffer_ptr const&, vertex_declaration_t const*, vertex_buffer_ptr const&, vertex_shader_ptr const&, fragment_shader_ptr const&) -> void;
		auto signal_draw_scene(scene_t&) -> void;
		
		auto signal_draw(shared_state_t const&, vertex_stage_state_t const&, fragment_stage_state_t const&) -> void;


		// resources
		auto signal_res_map(resource_ptr const&, uint subresource, map_type_t, map_callback_t const&) -> void;
		auto signal_res_update(constant_buffer_ptr const&, uint data_size, void*) -> void;
		auto signal_res_update(constant_buffer_ptr const&, atma::shared_memory_t const&) -> void;


		// geometry-stage
		auto signal_gs_upload_constant_buffer(uint index, constant_buffer_cptr const&) -> void;
		auto signal_gs_set(geometry_shader_ptr const&) -> void;


		// vertex-stage
		auto signal_vs_upload_constant_buffer(uint index, constant_buffer_cptr const&) -> void;


		// pixel-stage
		auto signal_fs_upload_shader_resource(uint index, resource_ptr const&) -> void;
		auto signal_fs_upload_constant_buffer(uint index, constant_buffer_cptr const&) -> void;


		// compute-stage
		auto signal_cs_set(compute_shader_ptr const&) -> void;
		auto signal_cs_upload_constant_buffer(uint index, constant_buffer_cptr const&) -> void;
		auto signal_cs_upload_shader_resource(view_type_t, shader_resource2d_ptr const&) -> void;
		auto signal_cs_dispatch(uint x, uint y, uint z) -> void;


		// d3d-specific
		auto signal_d3d_buffer_upload(platform::d3d_buffer_ptr const&, void const* data, uint row_pitch, uint depth_pitch) -> void;
		
		auto d3d_device() const -> platform::d3d_device_ptr const& { return d3d_device_; }
		auto d3d_immediate_context() const -> platform::d3d_context_ptr const& { return d3d_immediate_context_; }

	private:
		context_t(runtime_t&, fooey::window_ptr const&, uint adapter);

		auto bind_events(fooey::window_ptr const&) -> void;
		auto create_swapchain() -> void; 
		auto setup_rendertarget(uint width, uint height) -> void;
		auto recreate_backbuffer() -> void;
		auto create_d3d_input_layout(vertex_shader_ptr const&, vertex_declaration_t const*) -> platform::d3d_input_layout_ptr;

		auto pull_display_format(display_mode_t&, DXGI_SWAP_CHAIN_DESC&) -> void;
		auto push_display_format(DXGI_MODE_DESC&, display_mode_t const&) -> void;

		auto update_display_mode() -> void;

		// these functions are called on a fooey thread
		auto on_resize(fooey::events::resize_t&) -> void;

	private:
		atma::thread::engine_t engine_;
		runtime_t& runtime_;

		platform::dxgi_adapter_ptr dxgi_adapter_;
		platform::dxgi_swap_chain_ptr dxgi_swap_chain_;
		platform::dxgi_output_ptr dxgi_output_;

		platform::d3d_device_ptr d3d_device_;
		platform::d3d_context_ptr d3d_immediate_context_;
		platform::d3d_context_ptr d3d_deferred_context_;
		platform::d3d_render_target_view_ptr d3d_render_target_;
		platform::d3d_depth_stencil_buffer_ptr d3d_depth_stencil_;
		platform::d3d_texture2d_ptr d3d_depth_stencil_buffer_;

		// fooey
		fooey::window_ptr window_;
		fooey::event_handler_t::delegate_set_t bound_events_;

		// our current display format (windowed), current display format (fullscreen),
		// which display mode we're currently at, the requested modes for both windowed
		// and fullscreen, and which mode we should transition to (changing between
		// windowed/fullscreen takes precedence over windowed/windowed size changes).
		display_mode_t windowed_display_mode_, fullscreen_display_mode_;
		display_mode_t* current_display_mode_;
		display_mode_t requested_windowed_display_mode_, requested_fullscreen_display_mode_;
		display_mode_t* requested_display_mode_;


		// cache for vertex-layouts
		std::map<std::tuple<vertex_shader_ptr, vertex_declaration_t const*>, platform::d3d_input_layout_ptr> cached_input_layouts_;

		friend auto create_context(runtime_t&, fooey::window_ptr const&, uint adapter) -> context_ptr;
	};

}

