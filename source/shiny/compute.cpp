#include <shiny/compute.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using queue_t = atma::thread::engine_t::queue_t;


auto shiny::detail::generate_compute_command(queue_t::batch_t& batch, context_ptr const& ctx, cs_execute_t const& ex) -> void
{
	batch.push([ctx, ex](){
		ctx->immediate_cs_set_compute_shader(ex.shader);
		ctx->immediate_cs_set_dispatch_ranges(ex.x, ex.y, ex.z);
	});
}

auto shiny::signal_compute(context_ptr const& ctx, atma::thread::engine_t::queue_t::batch_t& batch) -> void
{
	batch.push([ctx](){
		ctx->immediate_compute();
	});
}

