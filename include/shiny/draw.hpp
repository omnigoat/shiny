#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/thread/engine.hpp>


//
// stages / commands
//
namespace shiny
{
	struct input_assembly_stage_t
	{
		data_declaration_t const* dd = nullptr;
		topology_t tp                = topology_t::triangle;
		vertex_buffer_cptr vb;
		index_buffer_cptr ib;
	};

	struct geometry_stage_t
	{
		geometry_shader_cptr gs;
	};

	struct vertex_stage_t
	{
		vertex_shader_cptr vs;
		bound_constant_buffers_t cbs;
		bound_resources_t brs;
	};

	struct fragment_stage_t
	{
		fragment_shader_cptr fs;
		bound_constant_buffers_t cbs;
		bound_resources_t brs;
	};

	struct draw_range_t
	{
		draw_range_t()
			: index_offset(), index_count(), vertex_offset(), vertex_count()
		{}

		draw_range_t(uint io, uint ic, uint vo, uint vc)
			: index_offset(io), index_count(ic), vertex_offset(vo), vertex_count(vc)
		{}

		uint index_offset;
		uint index_count;
		uint vertex_offset;
		uint vertex_count;
	};


	namespace draw_commands
	{
		namespace detail
		{
			inline auto ia_set(input_assembly_stage_t& ia, data_declaration_t const* dd) -> void { ia.dd = dd; }
			inline auto ia_set(input_assembly_stage_t& ia, topology_t tp) -> void                { ia.tp = tp; }
			inline auto ia_set(input_assembly_stage_t& ia, vertex_buffer_cptr const& vb) -> void { ia.vb = vb; }
			inline auto ia_set(input_assembly_stage_t& ia, index_buffer_cptr const& ib) -> void  { ia.ib = ib; }

			inline auto gs_set(geometry_stage_t& gs, geometry_shader_cptr const& gsh) -> void { gs.gs = gsh; }

			inline auto vs_set(vertex_stage_t& vs, vertex_shader_cptr const& vsh) -> void       { vs.vs = vsh; }
			inline auto vs_set(vertex_stage_t& vs, bound_constant_buffers_t const& cbs) -> void { vs.cbs = cbs; }
			inline auto vs_set(vertex_stage_t& vs, bound_resources_t const& brs) -> void        { vs.brs = brs; }

			inline auto fs_set(fragment_stage_t& fs, fragment_shader_cptr const& fsh) -> void     { fs.fs = fsh; }
			inline auto fs_set(fragment_stage_t& fs, bound_constant_buffers_t const& cbs) -> void { fs.cbs = cbs; }
			inline auto fs_set(fragment_stage_t& fs, bound_resources_t const& brs) -> void        { fs.brs = brs; }
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
		using queue_t = atma::thread::engine_t::queue_t;

		auto generate_draw_prelude(queue_t::batch_t&, context_ptr const&) -> void;
		auto generate_command(queue_t::batch_t&, context_ptr const&, input_assembly_stage_t const&) -> void;
		auto generate_command(queue_t::batch_t&, context_ptr const&, geometry_stage_t const&) -> void;
		auto generate_command(queue_t::batch_t&, context_ptr const&, vertex_stage_t const&) -> void;
		auto generate_command(queue_t::batch_t&, context_ptr const&, fragment_stage_t const&) -> void;
		auto generate_command(queue_t::batch_t&, context_ptr const&, draw_range_t const&) -> void;
	}

	auto signal_draw(context_ptr const&, atma::thread::engine_t::queue_t::batch_t&) -> void;

	template <typename T, typename... Args>
	inline auto signal_draw(context_ptr const& ctx, atma::thread::engine_t::queue_t::batch_t& batch, T&& t, Args&&... args) -> void
	{
		detail::generate_command(batch, ctx, std::forward<T>(t));
		signal_draw(ctx, batch, std::forward<Args>(args)...);
	}

	template <typename... Args>
	inline auto signal_draw(context_ptr const& ctx, Args&&... args) -> void
	{
		atma::thread::engine_t::queue_t::batch_t batch;
		detail::generate_draw_prelude(batch, ctx);
		signal_draw(ctx, batch, std::forward<Args>(args)...);
	}
}