#include <shiny/draw.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using batch_t = atma::thread::engine_t::batch_t;


auto shiny::detail::generate_draw_prelude(batch_t& batch, context_ptr const& ctx) -> void
{
	batch.push([ctx]{
		ctx->immediate_draw_pipeline_reset();
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, input_assembly_stage_t const& ias) -> void
{
	batch.push([ctx, ias]
	{
		ctx->immediate_ia_set_topology(ias.tp);
		ctx->immediate_ia_set_data_declaration(ias.dd);
		ctx->immediate_ia_set_vertex_buffer(ias.vb);

		if (ias.ib)
		{
			ctx->immediate_ia_set_index_buffer(ias.ib);
		}
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, geometry_stage_t const& gs) -> void
{
	batch.push([ctx, gs]{
		ctx->immediate_gs_set_geometry_shader(gs.gs);
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, vertex_stage_t const& vs) -> void
{
	batch.push([ctx, vs]{
		ctx->immediate_vs_set_vertex_shader(vs.vs);
		ctx->immediate_vs_set_constant_buffers(vs.cbs);
		ctx->immediate_vs_set_input_views(vs.ivs);
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, fragment_stage_t const& fs) -> void
{
	batch.push([ctx, fs]{
		ctx->immediate_fs_set_fragment_shader(fs.fs);
		ctx->immediate_fs_set_constant_buffers(fs.cbs);
		ctx->immediate_fs_set_input_views(fs.ivs);
		ctx->immediate_fs_set_compute_views(fs.cvs);
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, output_merger_stage_t const& om) -> void
{
	batch.push([ctx, om]{
		ctx->immediate_om_set_blending(om.b);
	});
}

auto shiny::detail::generate_command(batch_t& batch, context_ptr const& ctx, draw_range_t const& dr) -> void
{
	batch.push([ctx, dr]{
		ctx->immediate_draw_set_range(dr);
	});
}

auto shiny::detail::dispatch_signal_draw(context_ptr const& ctx, batch_t& batch) -> void
{
	ctx->signal(batch);
}




auto shiny::signal_draw(context_ptr const& ctx, batch_t& batch) -> void
{
	batch.push([ctx]{
		ctx->immediate_draw();
	});
}
