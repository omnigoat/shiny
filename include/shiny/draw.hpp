#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/depth_state.hpp>

#include <atma/thread/engine.hpp>


//
// stages / commands
//
namespace shiny
{
	struct input_assembly_stage_t;
	struct geometry_stage_t;
	struct vertex_stage_t;
	struct fragment_stage_t;
	struct output_merger_stage_t;
	struct draw_range_t;
	struct draw_command_t;

	namespace detail
	{
		inline auto ia_set(input_assembly_stage_t& ia, data_declaration_t const* dd) -> void;
		inline auto ia_set(input_assembly_stage_t& ia, topology_t tp) -> void;
		inline auto ia_set(input_assembly_stage_t& ia, vertex_buffer_cptr const& vb) -> void;
		inline auto ia_set(input_assembly_stage_t& ia, index_buffer_cptr const& ib) -> void;

		inline auto vs_set(vertex_stage_t& vs, vertex_shader_handle const& vsh) -> void;
		inline auto vs_set(vertex_stage_t& vs, bound_constant_buffers_t const& cbs) -> void;
		inline auto vs_set(vertex_stage_t& vs, bound_input_views_t const& ivs) -> void;

		inline auto gs_set(geometry_stage_t& gs, geometry_shader_cptr const& gsh) -> void;
		inline auto gs_set(geometry_stage_t& gs, bound_constant_buffers_t const& cbs) -> void;
		inline auto gs_set(geometry_stage_t& gs, bound_input_views_t const& ivs) -> void;

		inline auto fs_set(fragment_stage_t& fs, fragment_shader_handle const& fsh) -> void;
		inline auto fs_set(fragment_stage_t& fs, bound_constant_buffers_t const& cbs) -> void;
		inline auto fs_set(fragment_stage_t& fs, bound_input_views_t const& ivs) -> void;
		inline auto fs_set(fragment_stage_t& fs, bound_compute_views_t const& cvs) -> void;

		inline auto om_set(output_merger_stage_t& om, blender_cptr const& b) -> void;
		inline auto om_set(output_merger_stage_t& om, depth_stencil_state_t const& dss) -> void;
	}
}

namespace shiny
{

	struct input_assembly_stage_t
	{
		template <typename... Args>
		input_assembly_stage_t(Args&&... args) {
			ATMA_SPLAT_FN(detail::ia_set(*this, std::forward<Args>(args)));
		}

		data_declaration_t const* dd = nullptr;
		topology_t tp                = topology_t::triangle;
		vertex_buffer_cptr vb;
		index_buffer_cptr ib;
	};

	struct geometry_stage_t
	{
		template <typename... Args>
		geometry_stage_t(Args&&... args) {
			ATMA_SPLAT_FN(detail::gs_set(*this, std::forward<Args>(args)));
		}

		geometry_shader_cptr gs;
		bound_constant_buffers_t cbs;
		bound_input_views_t ivs;
	};

	struct vertex_stage_t
	{
		template <typename... Args>
		vertex_stage_t(Args&&... args) {
			ATMA_SPLAT_FN(detail::vs_set(*this, std::forward<Args>(args)));
		}

		vertex_shader_handle vs;
		bound_constant_buffers_t cbs;
		bound_input_views_t ivs;
	};

	struct fragment_stage_t
	{
		template <typename... Args>
		fragment_stage_t(Args&&... args) {
			ATMA_SPLAT_FN(detail::fs_set(*this, std::forward<Args>(args)));
		}

		fragment_shader_handle fs;
		bound_constant_buffers_t cbs;
		bound_input_views_t ivs;
		bound_compute_views_t cvs;
	};

	struct output_merger_stage_t
	{
		template <typename... Args>
		output_merger_stage_t(Args&&... args) {
			ATMA_SPLAT_FN(detail::om_set(*this, std::forward<Args>(args)));
		}

		blender_cptr b;
		depth_stencil_state_t dss = depth_stencil_state_t::standard;
	};

	struct draw_range_t
	{
		uint index_offset = 0;
		uint index_count = 0;
		uint vertex_offset = 0;
		uint vertex_count = 0;
	};

	struct draw_command_t
	{
		input_assembly_stage_t ia;
		geometry_stage_t gs;
		vertex_stage_t vs;
		fragment_stage_t fs;
		output_merger_stage_t om;
		draw_range_t dr;
	};

	using draw_commands_t = atma::vector<draw_command_t>;


	namespace detail
	{
		inline auto ia_set(input_assembly_stage_t& ia, data_declaration_t const* dd) -> void { ia.dd = dd; }
		inline auto ia_set(input_assembly_stage_t& ia, topology_t tp) -> void { ia.tp = tp; }
		inline auto ia_set(input_assembly_stage_t& ia, vertex_buffer_cptr const& vb) -> void { ia.vb = vb; }
		inline auto ia_set(input_assembly_stage_t& ia, index_buffer_cptr const& ib) -> void { ia.ib = ib; }

		inline auto vs_set(vertex_stage_t& vs, vertex_shader_handle const& vsh) -> void { vs.vs = vsh; }
		inline auto vs_set(vertex_stage_t& vs, bound_constant_buffers_t const& cbs) -> void { vs.cbs = cbs; }
		inline auto vs_set(vertex_stage_t& vs, bound_input_views_t const& ivs) -> void { vs.ivs = ivs; }

		inline auto gs_set(geometry_stage_t& gs, geometry_shader_cptr const& gsh) -> void { gs.gs = gsh; }
		inline auto gs_set(geometry_stage_t& gs, bound_constant_buffers_t const& cbs) -> void { gs.cbs = cbs; }
		inline auto gs_set(geometry_stage_t& gs, bound_input_views_t const& ivs) -> void { gs.ivs = ivs; }

		inline auto fs_set(fragment_stage_t& fs, fragment_shader_handle const& fsh) -> void { fs.fs = fsh; }
		inline auto fs_set(fragment_stage_t& fs, bound_constant_buffers_t const& cbs) -> void { fs.cbs = cbs; }
		inline auto fs_set(fragment_stage_t& fs, bound_input_views_t const& ivs) -> void { fs.ivs = ivs; }
		inline auto fs_set(fragment_stage_t& fs, bound_compute_views_t const& cvs) -> void { fs.cvs = cvs; }

		inline auto om_set(output_merger_stage_t& om, blender_cptr const& b) -> void { om.b = b; }
		inline auto om_set(output_merger_stage_t& om, depth_stencil_state_t const& dss) -> void { om.dss = dss; }
	}


	namespace draw_commands
	{
		namespace detail
		{
			inline auto ia_set(input_assembly_stage_t& ia, data_declaration_t const* dd) -> void { ia.dd = dd; }
			inline auto ia_set(input_assembly_stage_t& ia, topology_t tp) -> void                { ia.tp = tp; }
			inline auto ia_set(input_assembly_stage_t& ia, vertex_buffer_cptr const& vb) -> void { ia.vb = vb; }
			inline auto ia_set(input_assembly_stage_t& ia, index_buffer_cptr const& ib) -> void  { ia.ib = ib; }

			inline auto vs_set(vertex_stage_t& vs, vertex_shader_handle const& vsh) -> void       { vs.vs = vsh; }
			inline auto vs_set(vertex_stage_t& vs, bound_constant_buffers_t const& cbs) -> void   { vs.cbs = cbs; }
			inline auto vs_set(vertex_stage_t& vs, bound_input_views_t const& ivs) -> void        { vs.ivs = ivs; }

			inline auto gs_set(geometry_stage_t& gs, geometry_shader_cptr const& gsh) -> void     { gs.gs = gsh; }
			inline auto gs_set(geometry_stage_t& gs, bound_constant_buffers_t const& cbs) -> void { gs.cbs = cbs; }
			inline auto gs_set(geometry_stage_t& gs, bound_input_views_t const& ivs) -> void      { gs.ivs = ivs; }

			inline auto fs_set(fragment_stage_t& fs, fragment_shader_handle const& fsh) -> void   { fs.fs  = fsh; }
			inline auto fs_set(fragment_stage_t& fs, bound_constant_buffers_t const& cbs) -> void { fs.cbs = cbs; }
			inline auto fs_set(fragment_stage_t& fs, bound_input_views_t const& ivs) -> void      { fs.ivs = ivs; }
			inline auto fs_set(fragment_stage_t& fs, bound_compute_views_t const& cvs) -> void    { fs.cvs = cvs; }

			inline auto om_set(output_merger_stage_t& om, blender_cptr const& b) -> void { om.b = b; }
			inline auto om_set(output_merger_stage_t& om, depth_stencil_state_t const& dss) -> void { om.dss = dss; }
		}

		template <typename... Args>
		inline auto input_assembly_stage(Args&&... args) -> input_assembly_stage_t
		{
			input_assembly_stage_t ia;
			std::make_tuple((detail::ia_set(ia, std::forward<Args>(args)), 0)...);
			return ia;
		}

		template <typename... Args>
		inline auto geometry_stage(Args&&... args) -> geometry_stage_t
		{
			geometry_stage_t gs;
			std::make_tuple((detail::gs_set(gs, std::forward<Args>(args)), 0)...);
			return gs;
		}

		template <typename... Args>
		inline auto vertex_stage(Args&&... args) -> vertex_stage_t
		{
			vertex_stage_t vs;
			std::make_tuple((detail::vs_set(vs, std::forward<Args>(args)), 0)...);
			return vs;
		}

		template <typename... Args>
		inline auto fragment_stage(Args&&... args) -> fragment_stage_t
		{
			fragment_stage_t fs;
			std::make_tuple((detail::fs_set(fs, std::forward<Args>(args)), 0)...);
			return fs;
		}

		template <typename... Args>
		inline auto output_merger_stage(Args&&... args) -> output_merger_stage_t
		{
			output_merger_stage_t om;
			std::make_tuple((detail::om_set(om, std::forward<Args>(args)), 0)...);
			return om;
		}

		inline auto draw_range(uint offset, uint count) -> draw_range_t
		{
			return draw_range_t{0, 0, offset, count};
		}

		inline auto draw_indexed_range(uint offset, uint count, uint vertex_offset) -> draw_range_t
		{
			return draw_range_t{offset, count, vertex_offset, 0};
		}
	}
}



namespace shiny
{
	namespace detail
	{
		inline auto generate_command(draw_command_t& dc, input_assembly_stage_t const& ia) -> void { dc.ia = ia; }
		inline auto generate_command(draw_command_t& dc, geometry_stage_t const& gs) -> void { dc.gs = gs; }
		inline auto generate_command(draw_command_t& dc, vertex_stage_t const& vs) -> void { dc.vs = vs; }
		inline auto generate_command(draw_command_t& dc, fragment_stage_t const& fs) -> void { dc.fs = fs; }
		inline auto generate_command(draw_command_t& dc, output_merger_stage_t const& om) -> void { dc.om = om; }
		inline auto generate_command(draw_command_t& dc, draw_range_t const& dr) -> void { dc.dr = dr; }
	}

	template <typename... Args>
	inline auto signal_draw(renderer_ptr const& rndr, draw_commands_t& commands, Args&&... args) -> void
	{
		draw_command_t dc;
		ATMA_SPLAT_FN(detail::generate_command(dc, std::forward<Args>(args)));
		commands.push_back(dc);
	}

#if 0
	template <typename... Args>
	inline auto signal_draw(renderer_ptr const& rndr, Args&&... args) -> void
	{
		atma::thread::engine_t::queue_t::batch_t batch;
		signal_draw(rndr, batch, std::forward<Args>(args)...);
	}
#endif

}
