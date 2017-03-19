#include <shiny/platform/dx11/vertex_buffer.hpp>

#include <shiny/renderer.hpp>
#include <shiny/data_declaration.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::vertex_buffer_t;


vertex_buffer_t::vertex_buffer_t(renderer_ptr const& renderer, resource_storage_t usage, data_declaration_t const* vd, uint vertex_count, void const* data, uint data_vertcount)
	: buffer_t(renderer,
		resource_type_t::vertex_buffer,
		resource_usage_mask_t::none,
		usage,
		buffer_dimensions_t{vd->stride(), vertex_count},
		buffer_data_t{data, data_vertcount})
	, data_declaration_(vd), vertex_count_(vertex_count)
{
}

vertex_buffer_t::~vertex_buffer_t()
{
}
