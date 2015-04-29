#include <shiny/platform/win32/vertex_buffer.hpp>

#include <shiny/context.hpp>
#include <shiny/data_declaration.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::vertex_buffer_t;


vertex_buffer_t::vertex_buffer_t(context_ptr const& context, resource_storage_t usage, data_declaration_t const* vd, uint vertex_count, void const* data, uint data_vertcount)
	: buffer_t(context,
		resource_type_t::vertex_buffer,
		resource_usage_mask_t::none,
		usage,
		buffer_dimensions_t{vd->stride(), vertex_count},
		buffer_data_t::copy(data, data_vertcount))
	, data_declaration_(vd), vertex_count_(vertex_count)
{
}

vertex_buffer_t::~vertex_buffer_t()
{
}
