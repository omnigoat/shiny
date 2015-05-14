#include <shiny/compute.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using queue_t = atma::thread::engine_t::queue_t;


auto shiny::detail::generate_compute_prelude(queue_t::batch_t& batch, context_ptr const& ctx) -> void
{
	batch.push([ctx]{
		ctx->immediate_compute_pipeline_reset();
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, context_ptr const& ctx, bound_constant_buffers_t const& buffers) -> void
{
	batch.push([ctx, buffers]{
		ctx->immediate_cs_set_constant_buffers(buffers);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, context_ptr const& ctx, bound_input_views_t const& views) -> void
{
	batch.push([ctx, views]{
		ctx->immediate_cs_set_input_views(views.views);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, context_ptr const& ctx, bound_compute_views_t const& views) -> void
{
	batch.push([ctx, views]{
		ctx->immediate_cs_set_compute_views(views.views);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, context_ptr const& ctx, cs_dispatch_t const& ds) -> void
{
	batch.push([ctx, ds]{
		ctx->immediate_cs_set_compute_shader(ds.shader);
		ctx->immediate_compute(ds.x, ds.y, ds.z);
	});
}

auto shiny::signal_compute(context_ptr const& ctx, atma::thread::engine_t::queue_t::batch_t& batch) -> void
{
	ctx->signal(batch);
}

