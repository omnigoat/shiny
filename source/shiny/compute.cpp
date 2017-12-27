#include <shiny/compute.hpp>

#include <shiny/renderer.hpp>


using namespace shiny;
using queue_t = atma::thread::engine_t::queue_t;

auto shiny::signal_compute(renderer_ptr const& rndr, atma::thread::engine_t::queue_t::batch_t& batch) -> void
{
	rndr->signal(batch);
}

auto shiny::compute_context_t::signal_dispatch(compute_shader_cptr const& shader, uint32 dims) -> compute_context_t&
{
	return signal_dispatch(shader, dims, dims, dims);
}

auto shiny::compute_context_t::signal_dispatch(compute_shader_cptr const& shader, uint32 x, uint32 y, uint32 z) -> compute_context_t&
{
	rndr_->signal_compute(this, shader, x, y, z);
	return *this;
}

shiny::compute_context_t::~compute_context_t()
{}
