#include <shiny/platform/win32/generic_buffer.hpp>

#include <shiny/context.hpp>

using namespace shiny;
using shiny::generic_buffer_t;


generic_buffer_t::generic_buffer_t(context_ptr const& ctx, resource_usage_mask_t const& rs, buffer_usage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount)
	: buffer_t(ctx, resource_type_t::generic_buffer, rs, usage, stride, elements, data, data_elemcount)
{
}


auto shiny::make_generic_buffer(
	context_ptr const& ctx,
	resource_usage_mask_t rmask,
	buffer_usage_t usage,
	size_t stride, uint elements,
	void const* data, uint data_elements) -> generic_buffer_ptr
{
	return atma::make_intrusive_ptr<generic_buffer_t>(ctx, rmask, usage, stride, elements, data, data_elements);
}
