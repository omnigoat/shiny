#include <shiny/compute.hpp>

#include <shiny/renderer.hpp>


using namespace shiny;
using shiny::compute_context_t;

auto compute_context_t::signal_dispatch(compute_shader_cptr const& shader, uint32 dims) -> compute_context_t&
{
	return signal_dispatch(shader, dims, dims, dims);
}

auto compute_context_t::signal_dispatch(compute_shader_cptr const& shader, uint32 x, uint32 y, uint32 z) -> compute_context_t&
{
	rndr_->signal_compute(this, shader, x, y, z);
	return *this;
}

compute_context_t::~compute_context_t()
{}
