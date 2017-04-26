#include <shiny/platform/dx11/index_buffer.hpp>

#include <shiny/renderer.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::index_buffer_t;

#if 0
index_buffer_t::index_buffer_t(renderer_ptr const& renderer, resource_storage_t usage, format_t format, uint index_count, void const* data, uint data_indexcount)
	: buffer_t(renderer, resource_type_t::index_buffer, resource_usage_mask_t::none, usage, buffer_dimensions_t{element_size(format), index_count}, buffer_data_t{data, data_indexcount})
	, index_count_(index_count), index_format_(format)
{
}

index_buffer_t::~index_buffer_t()
{
}
#endif
