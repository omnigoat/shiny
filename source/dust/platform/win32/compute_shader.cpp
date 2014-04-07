#include <dust/compute_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::compute_shader_t;


auto dust::create_compute_shader(context_ptr const& context, void const* data, uint data_size) -> compute_shader_ptr
{
	return compute_shader_ptr(new compute_shader_t(context, data, data_size));
}

compute_shader_t::compute_shader_t(context_ptr const& context, void const* data, uint data_size)
: context_(context)
{
	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, context->d3d_device()->CreateComputeShader(data, data_size, nullptr, d3d_cs_.assign()));
}

