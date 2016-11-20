#include <shiny/compute.hpp>

#include <shiny/renderer.hpp>


using namespace shiny;
using queue_t = atma::thread::engine_t::queue_t;


auto shiny::detail::generate_compute_prelude(queue_t::batch_t& batch, renderer_ptr const& rndr) -> void
{
	batch.push([rndr]{
		rndr->immediate_compute_pipeline_reset();
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, renderer_ptr const& rndr, bound_constant_buffers_t const& buffers) -> void
{
	batch.push([rndr, buffers]{
		rndr->immediate_cs_set_constant_buffers(buffers);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, renderer_ptr const& rndr, bound_input_views_t const& views) -> void
{
	batch.push([rndr, views]{
		rndr->immediate_cs_set_input_views(views.views);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, renderer_ptr const& rndr, bound_compute_views_t const& views) -> void
{
	batch.push([rndr, views]{
		rndr->immediate_cs_set_compute_views(views.views);
	});
}

auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, renderer_ptr const& rndr, cs_dispatch_t const& ds) -> void
{
	batch.push([rndr, ds]{
		rndr->immediate_cs_set_compute_shader(ds.shader);
		rndr->immediate_compute(ds.x, ds.y, ds.z);
	});
}

auto shiny::signal_compute(renderer_ptr const& rndr, atma::thread::engine_t::queue_t::batch_t& batch) -> void
{
	rndr->signal(batch);
}

