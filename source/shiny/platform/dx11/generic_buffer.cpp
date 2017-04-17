#include <shiny/platform/dx11/generic_buffer.hpp>

#include <shiny/renderer.hpp>

using namespace shiny;
using shiny::generic_buffer_t;


generic_buffer_t::generic_buffer_t(renderer_ptr const& rndr, resource_usage_mask_t const& rs, resource_storage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount)
	: buffer_t(rndr, resource_type_t::generic_buffer, rs, usage, buffer_dimensions_t{stride, elements}, buffer_data_t{data, data_elemcount})
{
}


auto shiny::make_generic_buffer(
	renderer_ptr const& rndr,
	resource_usage_mask_t rmask,
	resource_storage_t usage,
	size_t stride, uint elements,
	void const* data, uint data_elements) -> generic_buffer_ptr
{
	return atma::make_intrusive<generic_buffer_t>(rndr, rmask, usage, stride, elements, data, data_elements);
}